#include "MediaWriterMp4.h"
#include <string.h>

GueeMediaWriterMp4::GueeMediaWriterMp4(GueeMediaStream& stream)
        : GueeMediaWriter(stream)
{
}
GueeMediaWriterMp4::~GueeMediaWriterMp4()
{

}
bool GueeMediaWriterMp4::onWriteHeader()
{
	const SVideoParams&	videoParams = m_stream.videoParams();
	const SAudioParams&	audioParams = m_stream.audioParams();
	bool	hasVideo = m_stream.hasVideo() && videoParams.enabled;
	bool	hasAudio = m_stream.hasAudio() && audioParams.enabled;

	m_lastWriteTrackId = 0;
	m_isAdtsAAC = audioParams.useADTS;

	set_ftyp("mp42isom");
    writeBox(m_boxRoot.find(FourCC('f','t','y','p')), true);
    writeBox(new Mp4Box(FourCC('f','r','e','e'), &m_boxRoot), true);
    m_mdatSizeOffset = uint32_t(m_totalBytes);
    writeBox(new Mp4Box(FourCC('m','d','a','t'), &m_boxRoot), true);
	set_moov_mvhd();
	if (hasVideo)
	{
		m_defaultVideoTrackId	= add_trak(eMedTypeVideo);
		TrackInfo&	trak = m_tracks[m_defaultVideoTrackId];
        trak.tkhd.width.u32_16.high = uint16_t(videoParams.width);
        trak.tkhd.height.u32_16.high = uint16_t(videoParams.height);
		trak.mdhd.timeScalc = 10000;
        Mp4Box*	avc1Box = new Mp4Box(FourCC('a','v','c','1'), trak.stsdBox, sizeof(SampleEntryVisual));
        SampleEntryVisual*	v	= reinterpret_cast<SampleEntryVisual*>(&avc1Box->data.front());
		v->dataReferenceIndex = endianFix16(1);;
        v->width = endianFix16(uint16_t(videoParams.width));
        v->height = endianFix16(uint16_t(videoParams.height));
		v->horizresolution = (72 << 8);
		v->vertresolution = (72 << 8);
		v->frame_count = endianFix16(1);
		v->depth = endianFix16(24);
		v->preDefined3 = int16_t(0xFFFF);

        const QByteArray& sps = m_stream.sps();
        const QByteArray& pps = m_stream.pps();
        Mp4Box*	avcCBox = new Mp4Box(FourCC('a','v','c','C'), avc1Box, uint32_t(6 + 2 + sps.size() + 1 + 2 + pps.size()));
        uint8_t*	data = reinterpret_cast<uint8_t*>(&avcCBox->data.front());
		data[0] = 1;
        data[1] = uint8_t(sps[1]);
        data[2] = uint8_t(sps[2]);
        data[3] = uint8_t(sps[3]);
		data[4] = 0xFF;
		data[5] = 0xE1;
		data += 6;
        *reinterpret_cast<uint16_t*>(data) = endianFix16(uint16_t(sps.size()));
		data += 2;
        memcpy(data, sps.data(), sps.size());
		data += sps.size();
		data[0] = 1;
		data++;
        *reinterpret_cast<uint16_t*>(data) = endianFix16(uint16_t(pps.size()));
		data += 2;
        memcpy(data, pps.data(), pps.size());

	}
	if (hasAudio)
	{
		m_defaultAudioTrackId	= add_trak(eMedTypeAudio);
		TrackInfo&	trak = m_tracks[m_defaultAudioTrackId];
        trak.mdhd.timeScalc = uint(audioParams.sampleRate);
        Mp4Box*	mp4aBox = new Mp4Box(FourCC('m','p','4','a'), trak.stsdBox, sizeof(SampleEntryAudio));
        SampleEntryAudio*	a = reinterpret_cast<SampleEntryAudio*>(&mp4aBox->data.front());
		a->dataReferenceIndex = endianFix16(1);
        a->channelCount = endianFix16(uint16_t(audioParams.channels));
        a->sampleSize = endianFix16(uint16_t(audioParams.sampleBits & 0xFFFF));
        a->sampleRate = endianFix32(uint16_t(audioParams.sampleRate));

		putByte(0);	//Box_FillHead
		putBE24(0);
		putByte(3);	//tag type: ES_DescrTag
		uint32_t	ES_DescrTagSizeOffset = cacheSize();
		putByte(0);	//ES_DescrTag Size
		putBE16(0);	//ES ID;
		putByte(0);	//ES Flags;

		putByte(4);	//tag type: DecoderConfigDescriptor
		uint32_t	DecoderConfigDescriptorSizeOffset = cacheSize();
		putByte(0);	//DecoderConfigDescriptor Size
		switch (audioParams.eCodec)	//objectTypeIndication
		{
		case AC_AAC:
			putByte(0x40);
			break;
		case AC_PCM:
			putByte(0);
			break;
		case AC_MP3:
			putByte(0x69);
			break;
		case AC_AC3:
			putByte(0xA5);
			break;
		case AC_DTS:
			putByte(0xA9);
			break;
		case AC_MP2AAC:
			if ( 1 == audioParams.encLevel )
				putByte(0x66);
			else if (2 == audioParams.encLevel)
				putByte(0x67);
			else if (3 == audioParams.encLevel)
				putByte(0x68);
			else
				putByte(0x66);
			break;
		default:
			putByte(0);
		}
		putByte(0x15);	//(streamType << 3) + (upStream << 1) + reserved(0)
		uint32_t	bitrateOffset = cacheSize();
		putBE24(680);	//bufferSizeDB 不知道怎么计算的，先写个固定值
        putBE32(audioParams.bitrate / audioParams.channels);	//maxBitrate;
        putBE32(audioParams.bitrate / audioParams.channels);	//avgBitrate;

		putByte(5);	//tag type: DecSpecificInfotag
		int32_t	decSpecSize = 0;
		m_stream.audioSpecificConfig(&decSpecSize);
		putByte(uint8_t(decSpecSize));
		appendData(m_stream.audioSpecificConfig(), decSpecSize);
		m_buffer[DecoderConfigDescriptorSizeOffset]	= uint8_t(cacheSize() - DecoderConfigDescriptorSizeOffset - 1);

		putByte(6);	//tag type: SLConfigDescrTag
		putByte(1);
		putByte(2);	//pre defined

		m_buffer[ES_DescrTagSizeOffset] = uint8_t(cacheSize() - ES_DescrTagSizeOffset - 1);

        Mp4Box*	esdsBox = new Mp4Box(FourCC('e','s','d','s'), mp4aBox, cacheSize());
		esdsBox->data = m_buffer;
		trak.esdsBitrateInfo = ((uint8_t*)&esdsBox->data.front()) + bitrateOffset;
		m_buffer.clear();
	}
    return true;
}
bool GueeMediaWriterMp4::onWriteVideo(const GueeMediaStream::H264Frame * frame)
{
	TrackInfo&	trak = m_tracks[m_defaultVideoTrackId];

	add_trak_stts(trak, uint32_t((frame->ptsTimeMS - trak.durationMs) * trak.mdhd.timeScalc / 1000), true);
	add_trak_ctts(trak, uint32_t((frame->ptsTimeMS - frame->dtsTimeMS) * trak.mdhd.timeScalc / 1000));
	trak.sampleCount++;
	trak.durationMs = frame->ptsTimeMS;
	if (trak.stssBox && frame->isKeyFrame)
	{
		trak.stssBox->data.append(4, 0);
        Box_stss *stss = reinterpret_cast<Box_stss*>(&trak.stssBox->data.front());
		stss->samples[stss->syncSample]	= trak.sampleCount;
		stss->syncSample++;
	}
	//string	dbg = to_string(trak.sampleCount);
	//dbg += frame->isKeyFrame ? "(Key): nal=" : ": nal=";
	//dbg	+= to_string(frame->nalCount);
    if ( m_stream.videoParams().annexb )
    {

        for ( int32_t i = 0; i < frame->nalCount; ++i )
        {
            const GueeMediaStream::H264Frame::NAL& nal = frame->nals[i];
			//dbg += (i == 0) ? "( " : ", ";
			//dbg += nal_unit_names[nal.nalType];
			//printf("%d : %d / %d, Type:%s\n", trak.sampleCount, i, frame->nalCount, nal_unit_names[nal.nalType]);
			//if ( nal.nalType == NalAud || nal.nalType == NalSps || nal.nalType == NalSei || nal.nalType == NalSps)
			//{
				//continue;
			//}
            int32_t	sizeAnnexb	= nal.nalData[2] == 1 ? 3 : 4;
            putBE32(nal.nalSize - sizeAnnexb );
            appendData(nal.nalData + sizeAnnexb, nal.nalSize - sizeAnnexb );
        }
    }
    else
    {
        for ( int32_t i = 0; i < frame->nalCount; ++i )
        {
            const GueeMediaStream::H264Frame::NAL& nal = frame->nals[i];
			//dbg += (i == 0) ? "( " : ", ";
			//dbg += nal_unit_names[nal.nalType];
			//if (nal.nalType == NalAud || nal.nalType == NalSps || nal.nalType == NalSei || nal.nalType == NalSps)
			//{
				//continue;
			//}
            appendData(nal.nalData, nal.nalSize);
        }
    }
	//dbg += " )\n";
	//printf(dbg.c_str());
	add_trak_stsc_stco_stsz(trak, m_totalBytes, cacheSize());
	trak.dataLength += cacheSize();
	return flushData();
}
bool GueeMediaWriterMp4::onWriteAudio(const GueeMediaStream::AUDFrame * frame)
{
	TrackInfo&	trak = m_tracks[m_defaultAudioTrackId];
	add_trak_stts(trak, 1024);
	trak.sampleCount++;
	trak.durationMs = frame->timestamp;
	if (m_isAdtsAAC)
	{
		appendData(frame->data + 7, frame->size - 7);
	}
	else
	{
		appendData(frame->data, frame->size);
	}
	add_trak_stsc_stco_stsz(trak, m_totalBytes, cacheSize());
	trak.dataLength += cacheSize();
    return flushData();
}

void GueeMediaWriterMp4::onCloseWrite()
{
	for (auto t = m_tracks.begin(); t != m_tracks.end(); ++t)
	{
		TrackInfo&	trak = t->second;
		if (trak.sampleCount)
		{
			if (trak.sttsBox)
			{
				Box_stts *stts = (Box_stts*)&trak.sttsBox->data.front();
				for (uint32_t i = 0; i < stts->timeToSample; ++i)
				{
					stts->samples[i].sampleCount = endianFix32(stts->samples[i].sampleCount);
					stts->samples[i].sampleDuration = endianFix32(stts->samples[i].sampleDuration);
				}
				stts->timeToSample = endianFix32(stts->timeToSample);
			}
			if (trak.cttsBox)
			{
                Box_ctts *ctts = reinterpret_cast<Box_ctts*>(&trak.cttsBox->data.front());
				for (uint32_t i = 0; i < ctts->timeToSample; ++i)
				{
					ctts->samples[i].sampleCount = endianFix32(ctts->samples[i].sampleCount);
					ctts->samples[i].sampleDuration = endianFix32(ctts->samples[i].sampleDuration);
				}
				ctts->timeToSample = endianFix32(ctts->timeToSample);
			}
			if (trak.stssBox)
			{
                Box_stss *stss = reinterpret_cast<Box_stss*>(&trak.stssBox->data.front());
				for (uint32_t i = 0; i < stss->syncSample; ++i)
				{
					stss->samples[i] = endianFix32(stss->samples[i]);
				}
				stss->syncSample = endianFix32(stss->syncSample);
			}
			if (trak.stscBox)
			{
                Box_stsc *stsc = reinterpret_cast<Box_stsc*>(&trak.stscBox->data.front());
				for (uint32_t i = 0; i < stsc->sampleToChunk; ++i)
				{
					stsc->chunks[i].firstChunk = endianFix32(stsc->chunks[i].firstChunk);
					stsc->chunks[i].sampleCount = endianFix32(stsc->chunks[i].sampleCount);
					stsc->chunks[i].descriptionId = endianFix32(stsc->chunks[i].descriptionId);
				}
				stsc->sampleToChunk = endianFix32(stsc->sampleToChunk);
			}
			if (trak.stszBox)
			{
                Box_stsz *stsz = reinterpret_cast<Box_stsz*>(&trak.stszBox->data.front());
				if (stsz->sampleSize)
					stsz->sampleSize = endianFix32(stsz->sampleSize);
				else
				{
					for (uint32_t i = 0; i < stsz->sampleCount; ++i)
					{
						stsz->sizes[i] = endianFix32(stsz->sizes[i]);
					}
				}
				stsz->sampleCount = endianFix32(stsz->sampleCount);
			}
			if (trak.stcoBox)
			{
				Box_stco *stco = (Box_stco*)&trak.stcoBox->data.front();
				for (uint32_t i = 0; i < stco->chunkCount; ++i)
				{
					stco->offsets[i] = endianFix32(stco->offsets[i]);
				}
				stco->chunkCount = endianFix32(stco->chunkCount);
			}
			if (trak.esdsBitrateInfo)
			{
				*((uint32_t*)trak.esdsBitrateInfo) = endianFix32(uint32_t(trak.dataLength / trak.sampleCount) << 8);
				*((uint32_t*)(trak.esdsBitrateInfo + 3)) = endianFix32(uint32_t(trak.dataLength * 1000 * 8 / trak.durationMs));
				*((uint32_t*)(trak.esdsBitrateInfo + 7)) = endianFix32(uint32_t(trak.dataLength * 1000 * 8 / trak.durationMs));
			}
		}
		trak.mdhd.duration = trak.durationMs * trak.mdhd.timeScalc / 1000;
		trak.tkhd.duration = trak.durationMs * m_mvhd.timeScalc / 1000;
		set_trak_tkhd(trak);
		set_mdia_mdhd(trak);
		if (m_mvhd.duration < trak.tkhd.duration) m_mvhd.duration = trak.tkhd.duration;


	}
	set_moov_mvhd();
	fix_mdat_size();
    writeBox(m_boxRoot.find(FourCC('m','o','o','v')), true);
}

bool GueeMediaWriterMp4::set_ftyp(const string& brands)
{
	int32_t brandCount = (int32_t)brands.size() / 4;
	if (brandCount == 0) return false;
    Mp4Box*	ftypBox = m_boxRoot.find(FourCC('f','t','y','p'));
	if (ftypBox)
	{
		ftypBox->data.resize(sizeof(Box_ftyp) + brandCount * 4 - 4);
	}
	else
	{
        ftypBox	= new Mp4Box(FourCC('f','t','y','p'), &m_boxRoot, sizeof(Box_ftyp) + brandCount * 4 - 4);
	}
	Box_ftyp*	ftypPtr = (Box_ftyp*)&ftypBox->data.front();
	memcpy(&ftypPtr->majorBrand, brands.c_str(), 4 );
	ftypPtr->minorVersion = 0x01000000;
	for (int32_t i = 0; i < brandCount; ++i)
	{
		memcpy(&ftypPtr->compatibleBrands[i], &brands[i * 4], 4);
	}
	return true;
}

bool GueeMediaWriterMp4::set_moov_mvhd()
{
    Mp4Box*	moovBox = m_boxRoot.find(FourCC('m','o','o','v'));
	if (nullptr == moovBox)
	{
        moovBox = new Mp4Box(FourCC('m','o','o','v'), &m_boxRoot);
	}
    Mp4Box*	mvhdBox = moovBox->find(FourCC('m','v','h','d'));
	if (nullptr == mvhdBox)
	{
        mvhdBox	= new Mp4Box(FourCC('m','v','h','d'), moovBox);
		m_mvhd.version = 1;
		m_mvhd.flags[0] = m_mvhd.flags[1] = m_mvhd.flags[2] = 0;
		m_mvhd.creationTime = 0;
		m_mvhd.modificationTime = 0;
		m_mvhd.timeScalc = 10000;
		m_mvhd.duration = 0;
        m_mvhd.rate.u32_16.high = 1;
        m_mvhd.rate.u32_16.low = 0;
        m_mvhd.volume.u16_8.high = 1;
        m_mvhd.volume.u16_8.low = 0;
		m_mvhd.matrix[0] = 256;
		m_mvhd.matrix[4] = 256;
		m_mvhd.matrix[8] = 64;
		m_mvhd.nextTrackId = 1;
	}
	if (m_mvhd.duration <= 0xFFFFFFFF)
	{
		mvhdBox->data.resize(sizeof(Box_mvhd32));
		Box_mvhd32* mvhdPtr = (Box_mvhd32*)&mvhdBox->data.front();
		mvhdPtr->version = 0;
		mvhdPtr->flags[0] = mvhdPtr->flags[1] = mvhdPtr->flags[2] = 0;
		mvhdPtr->creationTime = endianFix32((uint32_t)m_mvhd.creationTime);
		mvhdPtr->modificationTime = endianFix32((uint32_t)m_mvhd.modificationTime);
		mvhdPtr->timeScalc = endianFix32(m_mvhd.timeScalc);
		mvhdPtr->duration = endianFix32((uint32_t)m_mvhd.duration);
		mvhdPtr->rate = endianFix32(m_mvhd.rate);
		mvhdPtr->volume = endianFix16(m_mvhd.volume);
		memcpy(mvhdPtr->matrix, m_mvhd.matrix, sizeof(uint32_t) * 9);
		mvhdPtr->nextTrackId = endianFix32(m_mvhd.nextTrackId);
	}
	else
	{
		mvhdBox->data.resize(sizeof(Box_mvhd64));
		Box_mvhd64* mvhdPtr = (Box_mvhd64*)&mvhdBox->data.front();
		mvhdPtr->version = 1;
		mvhdPtr->flags[0] = mvhdPtr->flags[1] = mvhdPtr->flags[2] = 0;
		mvhdPtr->creationTime = endianFix64(m_mvhd.creationTime);
		mvhdPtr->modificationTime = endianFix64(m_mvhd.modificationTime);
		mvhdPtr->timeScalc = endianFix32(m_mvhd.timeScalc);
		mvhdPtr->duration = endianFix64(m_mvhd.duration);
		mvhdPtr->rate = endianFix32(m_mvhd.rate);
		mvhdPtr->volume = endianFix16(m_mvhd.volume);
		memcpy(mvhdPtr->matrix, m_mvhd.matrix, sizeof(uint32_t) * 9);
		mvhdPtr->nextTrackId = endianFix32(m_mvhd.nextTrackId);
	}
	return true;
}

uint32_t GueeMediaWriterMp4::add_trak(EMediaType type)
{
    Mp4Box*	moovBox = m_boxRoot.find(FourCC('m','o','o','v'));
	if (nullptr == moovBox) return 0;

	TrackInfo	info;

    info.trakBox = new Mp4Box(FourCC('t','r','a','k'), moovBox);
	set_trak_tkhd(info);

    Mp4Box*	mdiaBox = new Mp4Box(FourCC('m','d','i','a'), info.trakBox);
	set_mdia_mdhd(info);
    new Mp4Box(FourCC('h','d','l','r'), mdiaBox);
    Mp4Box*	minfBox = new Mp4Box(FourCC('m','i','n','f'), mdiaBox);
	switch (type)
	{
	case eMedTypeVideo:
        set_mdia_hdlr(info, FourCC('v','i','d','e'), "");
		set_minf_vmhd(info);
		break;
	case eMedTypeAudio:
        set_mdia_hdlr(info, FourCC('s','o','u','n'), "");
		set_minf_smhd(info);
		break;
	case eMedTypeText:
		set_mdia_hdlr(info, 0, "");
		break;
	default:
		set_mdia_hdlr(info, 0, "");
	}

    Mp4Box*	dinfBox = new Mp4Box(FourCC('d','i','n','f'), minfBox);

    Mp4Box*	drefBox = new Mp4Box(FourCC('d','r','e','f'), dinfBox, sizeof(Box_dref));
	((Box_dref*)&drefBox->data.front())->entryCount = endianFix32(1);
    Mp4Box* urlBox = new Mp4Box(FourCC('u','r','l',' '), drefBox, sizeof(Box_FillHead));
	((Box_FillHead*)&urlBox->data.front())->flags[2] = 1;

    Mp4Box*	stblBox = new Mp4Box(FourCC('s','t','b','l'), minfBox);

    info.stsdBox = new Mp4Box(FourCC('s','t','s','d'), stblBox, sizeof(Box_stsd));
	((Box_stsd*)&info.stsdBox->data.front())->entryCount = endianFix32(1);
    info.sttsBox = new Mp4Box(FourCC('s','t','t','s'), stblBox, sizeof(Box_stts) - 8);
	if (type == eMedTypeVideo)
	{
        info.cttsBox = new Mp4Box(FourCC('c','t','t','s'), stblBox, sizeof(Box_ctts) - 8);
	}
	if (type == eMedTypeVideo)
	{
        info.stssBox = new Mp4Box(FourCC('s','t','s','s'), stblBox, sizeof(Box_stss) - 4);
	}
    info.stscBox = new Mp4Box(FourCC('s','t','s','c'), stblBox, sizeof(Box_stsc) - 12);
    info.stszBox = new Mp4Box(FourCC('s','t','s','z'), stblBox, sizeof(Box_stsz) - 4);
    info.stcoBox = new Mp4Box(FourCC('s','t','c','o'), stblBox, sizeof(Box_stco) - 4 );
	m_tracks[info.tkhd.trackId] = info;
	return info.tkhd.trackId;
}

bool GueeMediaWriterMp4::set_trak_tkhd(TrackInfo& trak)
{
    Mp4Box*	tkhdBox = trak.trakBox->find(FourCC('t','k','h','d'));
	if (nullptr == tkhdBox)
	{
        tkhdBox	= new Mp4Box(FourCC('t','k','h','d'), trak.trakBox);
		trak.tkhd.version = 0;
		trak.tkhd.flags[0] = trak.tkhd.flags[1];
		trak.tkhd.flags[2] = 7;
		trak.tkhd.creationTime = 0;
		trak.tkhd.modificationTime = 0;
		trak.tkhd.trackId = m_mvhd.nextTrackId++;
		trak.tkhd.duration = 0;
		trak.tkhd.layer = 0;
		trak.tkhd.alternateGroup = 0;
		trak.tkhd.volume = 0;
		trak.tkhd.matrix[0] = 256;
		trak.tkhd.matrix[4] = 256;
		trak.tkhd.matrix[8] = 64;
		trak.tkhd.width = 0;
		trak.tkhd.height = 0;
	}
	if (trak.tkhd.duration <= 0xFFFFFFFF)
	{
		tkhdBox->data.resize(sizeof(Box_tkhd32));
		Box_tkhd32* tkhdPtr = (Box_tkhd32*)&tkhdBox->data.front();
		tkhdPtr->version = 0;
		tkhdPtr->flags[0] = trak.tkhd.flags[0];
		tkhdPtr->flags[1] = trak.tkhd.flags[1];
		tkhdPtr->flags[2] = trak.tkhd.flags[2];
		tkhdPtr->creationTime = endianFix32((uint32_t)trak.tkhd.creationTime);
		tkhdPtr->modificationTime = endianFix32((uint32_t)trak.tkhd.modificationTime);
		tkhdPtr->trackId = endianFix32(trak.tkhd.trackId);
		tkhdPtr->duration = endianFix32((uint32_t)trak.tkhd.duration);
		tkhdPtr->layer = endianFix16(trak.tkhd.layer);
		tkhdPtr->alternateGroup = endianFix16(trak.tkhd.alternateGroup);
		tkhdPtr->volume = endianFix16(trak.tkhd.volume);
		memcpy(tkhdPtr->matrix, trak.tkhd.matrix, sizeof(uint32_t) * 9);
		tkhdPtr->width = endianFix32(trak.tkhd.width);
		tkhdPtr->height = endianFix32(trak.tkhd.height);
	}
	else
	{
		tkhdBox->data.resize(sizeof(Box_tkhd64));
		Box_tkhd64* tkhdPtr = (Box_tkhd64*)&tkhdBox->data.front();
		tkhdPtr->version = 1;
		tkhdPtr->flags[0] = trak.tkhd.flags[0];
		tkhdPtr->flags[1] = trak.tkhd.flags[1];
		tkhdPtr->flags[2] = trak.tkhd.flags[2];
		tkhdPtr->creationTime = endianFix64(trak.tkhd.creationTime);
		tkhdPtr->modificationTime = endianFix64(trak.tkhd.modificationTime);
		tkhdPtr->trackId = endianFix32(trak.tkhd.trackId);
		tkhdPtr->duration = endianFix64(trak.tkhd.duration);
		tkhdPtr->layer = endianFix16(trak.tkhd.layer);
		tkhdPtr->alternateGroup = endianFix16(trak.tkhd.alternateGroup);
		tkhdPtr->volume = endianFix16(trak.tkhd.volume);
		memcpy(tkhdPtr->matrix, trak.tkhd.matrix, sizeof(uint32_t) * 9);
		tkhdPtr->width = endianFix32(trak.tkhd.width);
		tkhdPtr->height = endianFix32(trak.tkhd.height);
	}
	return true;
}

bool GueeMediaWriterMp4::set_mdia_mdhd(TrackInfo & trak)
{
    Mp4Box*	mdiaBox = trak.trakBox->find(FourCC('m','d','i','a'));
	if (nullptr == mdiaBox) return false;
    Mp4Box*	mdhdBox = mdiaBox->find(FourCC('m','d','h','d'));
	if (nullptr == mdhdBox)
	{
        mdhdBox = new Mp4Box(FourCC('m','d','h','d'), mdiaBox);
		trak.mdhd.version = 1;
		trak.mdhd.flags[0] = trak.tkhd.flags[1] = trak.mdhd.flags[2] = 0;
		trak.mdhd.creationTime = 0;
		trak.mdhd.modificationTime = 0;
		trak.mdhd.timeScalc = 0;
		trak.mdhd.duration = 0;
		trak.mdhd.language = 0;
	}
	if (trak.mdhd.duration <= 0xFFFFFFFF)
	{
		mdhdBox->data.resize(sizeof(Box_mdhd32));
		Box_mdhd32* mdhdPtr = (Box_mdhd32*)&mdhdBox->data.front();
		mdhdPtr->version = 0;
		mdhdPtr->flags[0] = trak.mdhd.flags[0];
		mdhdPtr->flags[1] = trak.mdhd.flags[1];
		mdhdPtr->flags[2] = trak.mdhd.flags[2];
		mdhdPtr->creationTime = endianFix32((uint32_t)trak.mdhd.creationTime);
		mdhdPtr->modificationTime = endianFix32((uint32_t)trak.mdhd.modificationTime);
		mdhdPtr->timeScalc = endianFix32(trak.mdhd.timeScalc);
		mdhdPtr->duration = endianFix32((uint32_t)trak.mdhd.duration);
		mdhdPtr->language = endianFix32(trak.mdhd.language);
	}
	else
	{
		mdhdBox->data.resize(sizeof(Box_mdhd64));
		Box_mdhd64* mdhdPtr = (Box_mdhd64*)&mdhdBox->data.front();
		mdhdPtr->version = 1;
		mdhdPtr->flags[0] = trak.mdhd.flags[0];
		mdhdPtr->flags[1] = trak.mdhd.flags[1];
		mdhdPtr->flags[2] = trak.mdhd.flags[2];
		mdhdPtr->creationTime = endianFix64(trak.mdhd.creationTime);
		mdhdPtr->modificationTime = endianFix64(trak.mdhd.modificationTime);
		mdhdPtr->timeScalc = endianFix32(trak.mdhd.timeScalc);
		mdhdPtr->duration = endianFix64(trak.mdhd.duration);
		mdhdPtr->language = endianFix32(trak.mdhd.language);
	}
	return true;
}

bool GueeMediaWriterMp4::set_mdia_hdlr(TrackInfo & trak, FourCC fcc, const string& name)
{
    Mp4Box*	mdiaBox = trak.trakBox->find(FourCC('m','d','i','a'));
	if (nullptr == mdiaBox) return false;
    Mp4Box*	hdlrBox = mdiaBox->find(FourCC('h','d','l','r'));
	if (nullptr == hdlrBox)
	{
        hdlrBox = new Mp4Box(FourCC('h','d','l','r'), mdiaBox);
	}

	hdlrBox->data.resize(sizeof(Box_hdlr)+ name.size());
	Box_hdlr*	hdlr = (Box_hdlr*)&hdlrBox->data.front();
	hdlr->version = 0;
	hdlr->flags[0] = hdlr->flags[1] = hdlr->flags[2] = 0;
	hdlr->handlerType = fcc;
	memcpy(hdlr->name, &hdlrBox->data.front(), name.size() + 1);
	return true;
}

bool GueeMediaWriterMp4::set_minf_vmhd(TrackInfo & trak, uint16_t mode, uint16_t r, uint16_t g, uint16_t b)
{
    Mp4Box*	minfBox = trak.trakBox->find(FourCC('m','i','n','f'));
	if (nullptr == minfBox) return false;
    Mp4Box*	vmhdBox = minfBox->find(FourCC('v','m','h','d'));
	if (nullptr == vmhdBox)
	{
        vmhdBox = new Mp4Box(FourCC('v','m','h','d'), minfBox, sizeof(Box_vmhd));
	}

	Box_vmhd*	vmhd = (Box_vmhd*)&vmhdBox->data.front();
	vmhd->version = 0;
	vmhd->flags[0] = vmhd->flags[1] = 0;
	vmhd->flags[2] = 1;
	vmhd->graphicsMode = endianFix16(mode);
	vmhd->opColor[0] = endianFix16(r);
	vmhd->opColor[1] = endianFix16(g);
	vmhd->opColor[2] = endianFix16(b);
	return true;
}

bool GueeMediaWriterMp4::set_minf_smhd(TrackInfo & trak, int8_t leftOrRight)
{
    Mp4Box*	minfBox = trak.trakBox->find(FourCC('m','i','n','f'));
	if (nullptr == minfBox) return false;
    Mp4Box*	smhdBox = minfBox->find(FourCC('s','m','h','d'));
	if (nullptr == smhdBox)
	{
        smhdBox = new Mp4Box(FourCC('s','m','h','d'), minfBox, sizeof(Box_smhd));
	}

	Box_smhd*	smhd = (Box_smhd*)&smhdBox->data.front();
	smhd->version = 0;
	smhd->flags[0] = smhd->flags[1] = smhd->flags[2] = 0;
    smhd->balance.u16_8.low = uint8_t(leftOrRight);
	return true;
}

void GueeMediaWriterMp4::add_trak_stts(TrackInfo& trak, uint32_t dur, bool isPre)
{
	if (trak.sttsBox)
	{
        Box_stts *stts = reinterpret_cast<Box_stts*>(&trak.sttsBox->data.front());
		if (isPre)
		{
			if (stts->timeToSample == 0)
			{
				trak.sttsBox->data.append(8, 0);
                stts = reinterpret_cast<Box_stts*>(&trak.sttsBox->data.front());
				stts->samples[stts->timeToSample].sampleCount = 1;
				stts->samples[stts->timeToSample].sampleDuration = dur;
				++stts->timeToSample;
			}
			else if (dur != stts->samples[stts->timeToSample - 1].sampleDuration)
			{
				if (stts->samples[stts->timeToSample - 1].sampleCount == 1)
				{
					stts->samples[stts->timeToSample - 1].sampleCount = 2;
					stts->samples[stts->timeToSample - 1].sampleDuration = dur;
				}
				else
				{
					--stts->samples[stts->timeToSample - 1].sampleCount;
					trak.sttsBox->data.append(8, 0);
                    stts = reinterpret_cast<Box_stts*>(&trak.sttsBox->data.front());
					stts->samples[stts->timeToSample].sampleCount = 2;
					stts->samples[stts->timeToSample].sampleDuration = dur;
					++stts->timeToSample;
				}
			}
			else
			{
				++stts->samples[stts->timeToSample - 1].sampleCount;
			}
		}
		else
		{
			if (stts->timeToSample == 0 ||
				dur != stts->samples[stts->timeToSample - 1].sampleDuration)
			{
				trak.sttsBox->data.append(8, 0);
                stts = reinterpret_cast<Box_stts*>(&trak.sttsBox->data.front());
				stts->samples[stts->timeToSample].sampleCount = 1;
				stts->samples[stts->timeToSample].sampleDuration = dur;
				++stts->timeToSample;
			}
			else
			{
				++stts->samples[stts->timeToSample - 1].sampleCount;
			}
		}



	}
}

void GueeMediaWriterMp4::add_trak_ctts(TrackInfo& trak, uint32_t dely)
{
	if (trak.cttsBox)
	{
        Box_ctts *ctts = reinterpret_cast<Box_ctts*>(&trak.cttsBox->data.front());
		if (ctts->timeToSample == 0 ||
			dely != ctts->samples[ctts->timeToSample - 1].sampleDuration)
		{
			trak.cttsBox->data.append(8, 0);
            ctts = reinterpret_cast<Box_ctts*>(&trak.cttsBox->data.front());
			ctts->samples[ctts->timeToSample].sampleCount = 1;
			ctts->samples[ctts->timeToSample].sampleDuration = dely;
			++ctts->timeToSample;
		}
		else
		{
			++ctts->samples[ctts->timeToSample - 1].sampleCount;
		}
	}
}


void GueeMediaWriterMp4::add_trak_stsc_stco_stsz(TrackInfo & trak, uint64_t offset, uint32_t size)
{
	//Box_stts *stts = (Box_stts*)&trak.sttsBox->data.front();
	//Box_stts *stts = (Box_stts*)&trak.sttsBox->data.front();
    //Box_ctts *ctts = reinterpret_cast<Box_ctts*>(&trak.cttsBox->data.front());
    //Box_stss *stss = reinterpret_cast<Box_stss*>(&trak.stssBox->data.front());
    //Box_stsc *stsc = reinterpret_cast<Box_stsc*>(&trak.stscBox->data.front());
    //Box_stsz *stsz = reinterpret_cast<Box_stsz*>(&trak.stszBox->data.front());
	//Box_stco *stco = (Box_stco*)&trak.stcoBox->data.front();


    Box_stsc *stsc = reinterpret_cast<Box_stsc*>(&trak.stscBox->data.front());
	if (m_lastWriteTrackId == trak.tkhd.trackId)
	{
		++stsc->chunks[stsc->sampleToChunk - 1].sampleCount;
	}
	else
	{
		m_lastWriteTrackId = trak.tkhd.trackId;

		trak.stcoBox->data.append(4, 0);
		Box_stco *stco = (Box_stco*)&trak.stcoBox->data.front();
		stco->offsets[stco->chunkCount] = (uint32_t)offset;
		++stco->chunkCount;

		if (stsc->sampleToChunk >= 2 &&
			stsc->chunks[stsc->sampleToChunk - 1].sampleCount == stsc->chunks[stsc->sampleToChunk - 2].sampleCount)
		{
			stsc->chunks[stsc->sampleToChunk - 1].firstChunk = stco->chunkCount;
			stsc->chunks[stsc->sampleToChunk - 1].sampleCount = 1;
		}
		else
		{
			trak.stscBox->data.append(12, 0);
            stsc = reinterpret_cast<Box_stsc*>(&trak.stscBox->data.front());
			stsc->chunks[stsc->sampleToChunk].firstChunk = stco->chunkCount;
			stsc->chunks[stsc->sampleToChunk].sampleCount = 1;
			stsc->chunks[stsc->sampleToChunk].descriptionId = 1;
			++stsc->sampleToChunk;
		}
	}
	if (trak.stszBox)
	{
        Box_stsz *stsz = reinterpret_cast<Box_stsz*>(&trak.stszBox->data.front());
		uint32_t saved = uint32_t(trak.stszBox->data.size() - (sizeof(Box_stsz) - 4)) / 4;
		if (0 == trak.sampleCount)
		{
			stsz->sampleSize = size;
		}
		else if (saved || stsz->sampleSize != size)
		{
			while (saved < stsz->sampleCount)
			{
				trak.stszBox->data.append(4, 0);
                stsz = reinterpret_cast<Box_stsz*>(&trak.stszBox->data.front());
				stsz->sizes[saved] = stsz->sampleSize;
				++saved;
			}
			trak.stszBox->data.append(4, 0);
            stsz = reinterpret_cast<Box_stsz*>(&trak.stszBox->data.front());
			stsz->sizes[stsz->sampleCount] = size;
		}
		++stsz->sampleCount;
	}
}

bool GueeMediaWriterMp4::fix_mdat_size()
{
    return reputAviDwordToFile(int32_t(m_mdatSizeOffset), endianFix32(uint32_t(m_totalBytes - m_mdatSizeOffset)));
}

bool GueeMediaWriterMp4::writeBox(Mp4Box * box, bool flush)
{
	if (flush) box->resetSize();
	if (box->head.largeSize > 0xFFFFFFFF)
	{
		putBE32(1);
		putBE32(box->head.type);
		putBE64(box->head.largeSize);
	}
	else
	{
		putBE32(box->head.size);
		putBE32(box->head.type);
	}
	if (!box->data.empty())
        appendData(reinterpret_cast<const uint8_t*>(box->data.c_str()), uint32_t(box->data.length()));
	if (flush && !flushData())
		return false;
	for (auto i = box->childs.begin(); i != box->childs.end(); ++i)
	{
		writeBox(*i);
	}
	return true;
}
