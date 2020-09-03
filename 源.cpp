
#include <iostream>

#include <opencv2\opencv.hpp>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat\avformat.h>
}


void IniFFMPEG() {



}
int main(int argc, char **argv)
{
	const AVCodec *codec;
	AVCodecParserContext *parser;
	SwsContext *sws;
	AVCodecContext *c = NULL;
	uint8_t* buffer_rgb = NULL;
	
	uint8_t *buffer_cace;
	uint8_t *data;
	size_t   data_size;
	int ret;
	AVPacket *pkt;
	buffer_cace = new uint8_t[8000];
	FILE *fs;


	pkt = av_packet_alloc();
	//av_init_packet(pkt);
	codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		return -1;
	}
	parser = av_parser_init(codec->id);
	if (!parser) {
		fprintf(stderr, "parser not found\n");
		return -2;
	}
	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return -2;
	}
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return -4;
	}


	
	int bytes = av_image_get_buffer_size(AV_PIX_FMT_NV12, 1280, 720,1);
	AVFrame *pFrameNV12 = av_frame_alloc();
	buffer_rgb = (uint8_t *)av_malloc(bytes);

	int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_BGR24, 1280, 720, 1);;
	AVFrame  *pFrameRGB = av_frame_alloc();
	pFrameRGB->width = 1280;
	pFrameRGB->height = 720;
	pFrameRGB->format = AV_PIX_FMT_BGR24;
	uint8_t *m_Buffer = (unsigned char *)av_malloc(bufferSize);
	cv::Mat img = cv::Mat::zeros(cv::Size(1280, 720), CV_8UC3);
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, img.data, AV_PIX_FMT_BGR24, 1280, 720, 1);

	//av_image_get_buffer_size();
	sws = sws_getContext(1280, 720, AVPixelFormat::AV_PIX_FMT_NV12, 1280, 720, AVPixelFormat::AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL,NULL);
	fs=fopen("6004create_frame.h265","rb");

	while (!feof(fs))
	{
		int ret= fread((char*)buffer_cace,1,8000,fs);
		
		if (ret<0)
		{
			return -5;
		}
		data_size = ret;
		while (data_size>0&&buffer_cace!=NULL)
		{
			ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
				buffer_cace, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		
			if (ret < 0) {
				fprintf(stderr, "Error while parsing\n");
				continue;
			}
			buffer_cace += ret;
			data_size -= ret;
			if (pkt->size == 0)
			{
				break;
			}
			int ret_send = avcodec_send_packet(c, pkt);
			if (ret_send<0)
			{
				continue;
			}
			int got_picture = avcodec_receive_frame(c, pFrameNV12); //got_picture = 0 success, a frame was returned
			if (got_picture<0)
			{
				break;
			}
			sws_scale(sws, pFrameNV12->data, pFrameNV12->linesize, 0, pFrameNV12->height, pFrameRGB->data, pFrameRGB->linesize);
			uint8_t *data = (uint8_t*)m_Buffer;
			cv::imshow("123", img);
			cv::waitKey(1);
		}
	}

	av_frame_free(&pFrameRGB);
	av_frame_free(&pFrameNV12);

	return 0;
}
