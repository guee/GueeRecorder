#include "MediaWriterFlv.h"

GueeMediaWriterFlv::GueeMediaWriterFlv(GueeMediaStream& stream)
    : GueeMediaWriter(stream)
{
	m_writeFramerateOffset	= 0;
	m_writeDurationOffset	= 0;
	m_writeFileSizeOffset	= 0;
	m_writeVideoRateOffset	= 0;
	m_videoDataTotalSize	= 0;
	m_aacTagHeader			= 0;
}

GueeMediaWriterFlv::~GueeMediaWriterFlv(void)
{
}

bool GueeMediaWriterFlv::onWriteHeader()
{
	const SVideoParams&	videoParams = m_stream.videoParams();
	const SAudioParams&	audioParams = m_stream.audioParams();
	bool	hasVideo = m_stream.hasVideo() && videoParams.enabled;
	bool	hasAudio = m_stream.hasAudio() && audioParams.enabled;
	m_writeFramerateOffset	= 0;
	m_writeDurationOffset	= 0;
	m_writeFileSizeOffset	= 0;
	m_writeVideoRateOffset	= 0;
	m_videoDataTotalSize	= 0;
	m_aacTagHeader			= 0;

	//写入 FLV Header
	putTag( "FLV" ); // Signature
	putByte( 1 );    // Version
	uint8_t	flag	= 0;
	if (hasVideo ) flag |= FLV_HEADER_FLAG_HASVIDEO;
	if (hasAudio ) flag |= FLV_HEADER_FLAG_HASAUDIO;
	putByte( flag );    // Video and Audio
	putBE32( 9 );    // DataOffset, FLV 头的长度
	putBE32( 0 );    // PreviousTagSize0

	putByte( FLV_TAG_TYPE_META );    // Tag Type "script data"
	uint32_t	dataLenOffset = cacheSize();
	putBE24( 0 ); // data length
	putBE24( 0 ); // timestamp
	putBE32( 0 ); // reserved

	putByte( AMF_DATA_TYPE_STRING );
	putAmfString( "onMetaData" );
	putByte( AMF_DATA_TYPE_MIXEDARRAY );

	int	mixedArrayCount	= 0;
	if (hasVideo)	mixedArrayCount	+= 7;
	if (hasAudio)	mixedArrayCount	+= 5;
	putBE32( mixedArrayCount );	// 数组元素的个数

	//写入描述视频和音频信息的数据。
	//■ duration: a DOUBLE indicating the total duration of the file in seconds.
	//■ width: a DOUBLE indicating the width of the video in pixels.
	//■ height: a DOUBLE indicating the height of the video in pixels.
	//■ videodatarate: a DOUBLE indicating the video bit rate in kilobits per second.
	//■ framerate: a DOUBLE indicating the number of frames per second.
	//■ videocodecid: a DOUBLE indicating the video codec ID used in the file (see “Video tags” on page 8 for available CodecID values).
	//■ audiosamplerate: a DOUBLE indicating the frequency at which the audio stream is replayed
	//■ audiosamplesize: a DOUBLE indicating the resolution of a single audio sample
	//■ stereo: a BOOL indicating whether the data is stereo
	//■ audiocodecid: a DOUBLE indicating the audio codec ID used in the file (see “Audio tags” on page 6 for available SoundFormat values)
	//■ filesize: a DOUBLE indicating the total size of the file in bytes.
	if (hasVideo)
	{
		//视频编码信息的7个属性
		putAmfString( "width" );
		putAmfDouble(videoParams.width );

		putAmfString( "height" );
		putAmfDouble(videoParams.height );

		putAmfString( "framerate" );
        if(videoParams.vfr ) //是否为可变帧率
		{
			//如果是可变帧率，那么就要在结束时计算帧率并写入，因此暂存文件中写入帧率值的位置。
			m_writeFramerateOffset = uint32_t(cacheSize() + m_totalBytes + 1 );
		}
        putAmfDouble( (double)videoParams.frameRate);
		putAmfString( "videocodecid" );
		putAmfDouble( FLV_CODECID_H264 );

		putAmfString( "duration" );
		m_writeDurationOffset = uint32_t(cacheSize() + m_totalBytes + 1 );
		putAmfDouble( 0 ); // written at end of encoding

		putAmfString( "filesize" );
		m_writeFileSizeOffset = uint32_t(cacheSize() + m_totalBytes + 1 );
		putAmfDouble( 0 ); // written at end of encoding

		putAmfString( "videodatarate" );
		m_writeVideoRateOffset = uint32_t(cacheSize() + m_totalBytes + 1 );
		putAmfDouble(videoParams.bitrate ); // written at end of encoding
	}
	FlvCodecId	flvCodecAudio = FLV_CODECID_PEPCM;
	if (hasAudio)
	{
		m_isAdtsAAC = audioParams.useADTS;
		//音频编码的4个属性
		putAmfString( "audiodatarate" );
        putAmfDouble(audioParams.bitrate / audioParams.channels );
		putAmfString( "audiosamplerate" );
        putAmfDouble(audioParams.sampleRate );
		//putAmfString("audiosamplesize");
		//putAmfDouble(audioParams.encInputBits / 8 * audioParams.channels);
		putAmfString( "stereo" );
		putAmfBool(audioParams.channels == 2 ? 1 : 0);
		putAmfString( "audiocodecid" );
		switch(audioParams.eCodec)
		{
		case AC_PCM:
			flvCodecAudio = FLV_CODECID_PEPCM;
			break;
		case AC_AAC:
			flvCodecAudio = FLV_CODECID_AAC;
			break;
		case AC_MP3:
			flvCodecAudio = FLV_CODECID_MP3;
			break;
        default:
            break;
		}
		putAmfDouble(flvCodecAudio);
	}
	putAmfString( "" );
	putByte( AMF_DATA_TYPE_OBJECT_END );

	uint32_t	tagLength	= cacheSize() - dataLenOffset;
	reputAmfBE24Length( tagLength - 10, dataLenOffset );
	putBE32( tagLength + 1 );	// tag length

	if (hasVideo)
	{
		putByte( FLV_TAG_TYPE_VIDEO );
		dataLenOffset	= cacheSize();	// needed for overwriting length
		putBE24( 0 ); // rewrite later
		putBE24( 0 ); // timestamp
		putByte( 0 ); // timestamp extended
		putBE24( 0 ); // StreamID - Always 0

		//IF TagType == 9 THEN VideoTagHeader
		putByte( FLV_FRAME_KEY );		// Frametype and CodecID
		//Type of video frame. The following values are defined:
		//1 = key frame (for AVC, a seekable frame)
		//2 = inter frame (for AVC, a non-seekable frame)
		//3 = disposable inter frame (H.263 only)
		//4 = generated key frame (reserved for server use only)
		//5 = video info/command frame
		//IF CodecID == 7 THEN AVCPacketType

		putByte( 0 );					// AVC sequence header
		//The following values are defined:
		//0 = AVC sequence header
		//1 = AVC NALU
		//2 = AVC end of sequence (lower level NALU sequence ender is not required or supported)
		//IF AVCPacketType == 1 THEN Composition time offset ELSE 0

		putBE24( 0 );					// composition time
		//See ISO 14496-12, 8.15.3 for an explanation of composition
		//times. The offset in an FLV file is always in milliseconds.
		//AVC sequence header	AVCDecoderConfigurationRecord结构，该结构在标准文档“ISO-14496-15 AVC file format”中有详细说明。
		putByte( 1 );      // version

		// SPS
		putByte(m_stream.sps()[1] ); // profile
		putByte(m_stream.sps()[2] ); // profile
		putByte(m_stream.sps()[3] ); // level
		putByte( 0xff );   // 6 bits reserved (111111) + 2 bits nal size length - 1 (11)
		putByte( 0xe1 );   // 3 bits reserved (111) + 5 bits number of sps (00001)
		//Write SPS
		putBE16((uint16_t)m_stream.sps().size() );
        appendData( (const uint8_t*)m_stream.sps().data(), (uint32_t)m_stream.sps().size() );

		//Write PPS
		putByte( 1 ); // number of pps
		putBE16((uint16_t)m_stream.pps().size() );
        appendData((const uint8_t*)m_stream.pps().data(), (uint32_t)m_stream.pps().size() );

		tagLength	= cacheSize() - dataLenOffset;
		reputAmfBE24Length( tagLength - 10, dataLenOffset );
		putBE32( tagLength + 1 );	// tag length
	}

	if (hasAudio)
	{
		putByte( FLV_TAG_TYPE_AUDIO );
		putBE24( 4 );	// data size
		putBE24( 0 ); // timestamp
		putByte( 0 ); // timestamp extended
		putBE24( 0 ); // StreamID - Always 0

		m_aacTagHeader	= 0;
        if (audioParams.sampleRate < 11025 )
			m_aacTagHeader	|= FLV_SAMPLERATE_SPECIAL;
        else if (audioParams.sampleRate < 22050 )
			m_aacTagHeader	|= FLV_SAMPLERATE_11025HZ;
        else if (audioParams.sampleRate < 44100 )
			m_aacTagHeader	|= FLV_SAMPLERATE_22050HZ;
		else
			m_aacTagHeader	|= FLV_SAMPLERATE_44100HZ;

		m_aacTagHeader	|= (audioParams.channels == 1 ? FLV_MONO : FLV_STEREO );
        m_aacTagHeader	|= ( (audioParams.sampleBits & 0xFF ) == 8 ? FLV_SAMPLESSIZE_8BIT : FLV_SAMPLESSIZE_16BIT );
		m_aacTagHeader	|= (flvCodecAudio);
		putByte( m_aacTagHeader );
		putByte( 0 );
		appendData(m_stream.audioSpecificConfig(), 2 );
		putBE32( 15 );		// Last tag size
	}
	return flushData();
}

bool GueeMediaWriterFlv::onWriteVideo(const GueeMediaStream::H264Frame * frame)
{
	const SVideoParams&	videoParams = m_stream.videoParams();
    //qDebug() <<"onWriteVideo timestamp:" << frame->ptsTimeMS;
    // A new frame - write packet header
	putByte( FLV_TAG_TYPE_VIDEO );
	uint32_t	dataLenOffset	= (uint32_t)m_buffer.size();	// needed for overwriting length
	putBE24( 0 ); // calculated later
    putBE24( uint32_t(frame->dtsTimeMS) );
    putByte( uint8_t(frame->dtsTimeMS >> 24 ) );
	putBE24( 0 );
	putByte(frame->isKeyFrame ? FLV_FRAME_KEY : FLV_FRAME_INTER);
	putByte( 1 ); // AVC NALU
	putBE24( uint32_t(frame->ptsTimeMS - frame->dtsTimeMS) );

    if ( 0 == m_stream.videoFrameCount() && !m_stream.sei().isEmpty() )
	{
		putBE32((uint32_t)m_stream.sei().size() );
        appendData((const uint8_t*)m_stream.sei().data(), (uint32_t)m_stream.sei().size() );
	}

	if (videoParams.annexb )
	{
		for ( int32_t i = 0; i < frame->nalCount; ++i )
		{
            const GueeMediaStream::H264Frame::NAL& nal = frame->nals[i];
			//if ( nal[i].i_type == NAL_AUD || nal[i].i_type == NAL_SPS || nal[i].i_type == NAL_SEI || nal[i].i_type == NAL_PPS ) continue;
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
			//if ( nal[i].i_type == NAL_AUD || nal[i].i_type == NAL_SPS || nal[i].i_type == NAL_SEI || nal[i].i_type == NAL_PPS ) continue;
			appendData(nal.nalData, nal.nalSize);
		}
	}
	uint32_t	tagLength	= cacheSize() - dataLenOffset;
	reputAmfBE24Length( tagLength - 10, dataLenOffset );
	putBE32( tagLength + 1 );	// tag length
	return flushData();
}

bool GueeMediaWriterFlv::onWriteAudio(const GueeMediaStream::AUDFrame * frame)
{
	putByte( FLV_TAG_TYPE_AUDIO );
    putBE24(frame->size + 2 + (m_isAdtsAAC ? - 7 : 0) );	// data size
    //qDebug() <<"onWriteAudio timestamp:" << frame->timestamp;
	putBE24( uint32_t(frame->timestamp & 0xFFFFFF) );
	putByte( uint8_t((frame->timestamp >> 24) & 0xFF ) );
	putBE24( 0 );
	putByte( m_aacTagHeader );
	putByte( 1 );	//1 = AAC raw
	if (m_isAdtsAAC)
	{
		appendData(frame->data + 7, frame->size - 7);
		putBE32(frame->size + 13 - 7);		// Last tag size
	}
	else
	{
		appendData(frame->data, frame->size);
		putBE32(frame->size + 13);		// Last tag size
	}
	return flushData();
}

void GueeMediaWriterFlv::onCloseWrite()
{
	while ( flushData() )
	{
		double total_duration = max( m_stream.videoDuration(), m_stream.audioDuration() ) / 1000.0;
		if ( total_duration > 0 )
		{
			if ( m_writeDurationOffset && !reputAmfDoubleToFile( m_writeDurationOffset, total_duration ) ) //修改时间
			{
				break;
			}
			if ( m_writeFramerateOffset &&
				!reputAmfDoubleToFile( m_writeFramerateOffset, (double)m_stream.videoFrameCount() / total_duration ) )
			{
				break;
			}
			if ( m_writeVideoRateOffset && 
				!reputAmfDoubleToFile( m_writeVideoRateOffset, m_stream.videoTotalSize() * 8 / ( total_duration * 1000 ) ) )//修改码率
			{
				break;
			}
		}
		if ( m_writeFileSizeOffset && !reputAmfDoubleToFile( m_writeFileSizeOffset, (double)m_totalBytes ) )//修改文件大小
		{
			break;
		}
		break;
	}
    GueeMediaWriter::onCloseWrite();//清除数据
}
