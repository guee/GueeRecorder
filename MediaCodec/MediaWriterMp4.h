#pragma once
#include "MediaWriter.h"
#include "mp4struct.h"

class CMediaWriterMp4 :
	public CMediaWriter, public Mp4Struct
{
public:
    CMediaWriterMp4(CMediaStream& stream);
    virtual ~CMediaWriterMp4();
protected:
	virtual bool onWriteHeader();
	virtual bool onWriteVideo(const CMediaStream::H264Frame * frame);
	virtual bool onWriteAudio(const CMediaStream::AUDFrame * frame);
	virtual	void onCloseWrite();
private:
	Mp4Box	m_boxRoot;
	struct TrackInfo
	{
		Mp4Box*		trakBox;

		Box_tkhd64	tkhd;
		Box_mdhd64	mdhd;

		Mp4Box	*stsdBox;
		Mp4Box	*sttsBox;
		Mp4Box	*cttsBox;
		Mp4Box	*stssBox;
		Mp4Box	*stscBox;
		Mp4Box	*stszBox;
		Mp4Box	*stcoBox;

		uint8_t	*esdsBitrateInfo;	//24bit bufferSizeDB, 32bit maxBitrate, 32bit avgBitrate

		uint64_t	dataLength;
		uint64_t	durationMs;		//当前轨道的总时长(毫秒)
		uint32_t	sampleCount;	//当前轨道的总帧数

		TrackInfo() {
			clear();
		}
		void clear() {
			memset(this, 0, sizeof(TrackInfo));
		}
	};
	map<uint32_t,TrackInfo>	m_tracks;
	Box_mvhd64	m_mvhd;
	uint32_t	m_defaultVideoTrackId;
	uint32_t	m_defaultAudioTrackId;
	uint32_t	m_lastWriteTrackId;
	uint32_t	m_mdatSizeOffset;

	bool set_ftyp(const string& brands);
	bool set_moov_mvhd();
	uint32_t add_trak(EMediaType type);
	bool set_trak_tkhd(TrackInfo& trak);
	bool set_mdia_mdhd(TrackInfo& trak);
	bool set_mdia_hdlr(TrackInfo& trak, FourCC fcc, const string& name);
	bool set_minf_vmhd(TrackInfo& trak, uint16_t mode = 0, uint16_t r = 0, uint16_t g = 0, uint16_t b = 0);
	bool set_minf_smhd(TrackInfo& trak, int8_t leftOrRight = 0 );
	void add_trak_stts(TrackInfo& trak, uint32_t dur, bool isPre = false);
	void add_trak_ctts(TrackInfo& trak, uint32_t dely);
	void add_trak_stsc_stco_stsz(TrackInfo& trak, uint64_t offset, uint32_t size);
	bool fix_mdat_size();
	bool writeBox(Mp4Box* box, bool flush = false);
};
