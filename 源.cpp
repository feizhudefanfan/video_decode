
#include <iostream>
#include <WinSock2.h>
#include <opencv2\opencv.hpp>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat\avformat.h>
}
#pragma comment(lib,"ws2_32.lib")
#define SW_DECODE 1                 //1:软件解码     0：硬件解码
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
	AVPacket *pkt;
	buffer_cace = new uint8_t[20000];
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


	AVFrame *pFrameNV12 = av_frame_alloc();
	int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_BGR24, 1280, 720,1);
	AVFrame  *pFrameRGB = av_frame_alloc();
	pFrameRGB->width = 1280;
	pFrameRGB->height = 720;
	pFrameRGB->format = AV_PIX_FMT_BGR24;
	uint8_t *m_Buffer = (unsigned char *)av_malloc(bufferSize);
	//cv::Mat img = cv::Mat::zeros(cv::Size(1280, 720), CV_8UC3);
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, m_Buffer, AV_PIX_FMT_BGR24, 1280, 720,1);
	sws = sws_getContext(1280, 720, AVPixelFormat::AV_PIX_FMT_YUV420P, 1280, 720, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL,NULL);
	fs=fopen("6004create_frame.h265","rb");


	WSADATA mWasData;
	SOCKET mSocket;
	WSAStartup(MAKEWORD(2, 2), &mWasData);
ReConnnect:
	mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mSocket == INVALID_SOCKET) {

		closesocket(mSocket);
		goto ReConnnect;
	}
	int recvTimeout = 30 * 1000;  //30s接收超时时间设置
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout, sizeof(int))) {


		closesocket(mSocket);
		goto ReConnnect;

	}
	SOCKADDR_IN addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(6003);
	addr.sin_addr.S_un.S_addr = inet_addr("192.168.0.128");//点分十进制转ip地址
	int val = ::connect(mSocket, (SOCKADDR*)&addr, sizeof(addr));
	int ret;
	
	do
	{
		//int ret= fread((char*)buffer_cace,1,8000,fs);
		
		ret = ::recv(mSocket,(char*)buffer_cace,20000,0);
		if (ret<0)
		{
			continue;
		}
		data_size = ret;
		while (data_size>0)
		{
			
			uint8_t *bufefef = buffer_cace;
			int ret_ps = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
				(const uint8_t*)bufefef, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		
			if (ret_ps < 0) {
				fprintf(stderr, "Error while parsing\n");
				continue;
			}
			bufefef += ret_ps;
			data_size -= ret_ps;
			
			if (pkt->size == 0)
			{
				break;
			}
			int ret_send = avcodec_send_packet(c, pkt);
			if (ret_send<0)
			{
				break;
			}
			//memset(buffer_cace,0, data_size);
			int got_picture = avcodec_receive_frame(c, pFrameNV12); //got_picture = 0 success, a frame was returned
			if (got_picture<0)
			{
				break;
			}
#if SW_DECODE



			sws_scale(sws, pFrameNV12->data, pFrameNV12->linesize, 0, pFrameNV12->height, pFrameRGB->data, pFrameRGB->linesize);
#else

#endif // SW_DECODE
			uint8_t *data = m_Buffer;
			
			cv::Mat img = cv::Mat::zeros(cv::Size(1280,720),CV_8UC3);
			memcpy(img.data, data,1280*720*3);
			cv::imshow("123", img);
			cv::waitKey(1);
			

		}
		
	}while (ret > 0);

	av_frame_free(&pFrameRGB);
	av_frame_free(&pFrameNV12);

	return 0;
}
