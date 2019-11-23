// 1-extractYUV420.cpp : Defines the entry point for the application.
//

#include "1-extractYUV420.h"
#include <string>
#include <fstream>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

using namespace std;

int main(int argc, char* argv[])
{
	cout << avcodec_configuration() << endl;

    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <src file>\n";
        exit(-1);
    }

    av_register_all();

    AVFormatContext *fmt_ctx = avformat_alloc_context();
    if (fmt_ctx == nullptr) {
        cerr << "avformat_alloc_context -> nullptr\n";
        return -1;
    }

    int ret = 0;
    ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret != 0) {
        cerr << "avformat_open_input ->" << ret << endl;
        return ret;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        cerr << "avformat_open_input ->" << ret << endl;
        return ret;
    }

    av_dump_format(fmt_ctx, 0, argv[1], 0);

    int target_stream_idx = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; ++i) {
        if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            target_stream_idx = i;
            break;
        }
    }

    if (target_stream_idx == -1) {
        cerr << "No video stream found." << ret << endl;
        return -1;
    }

    AVCodecContext *dec_ctx = fmt_ctx->streams[target_stream_idx]->codec;
    AVCodec *dec = avcodec_find_decoder(dec_ctx->codec_id);

    ret = avcodec_open2(dec_ctx, dec, nullptr);
    if (ret < 0) {
        cerr << "avcodec_open2 failed." << ret << endl;
        return ret;
    }
  
    int v_frame_idx = 0;
    AVFrame* frame = av_frame_alloc();

    // YUV420 frame buffer filling
    AVFrame* frame_YUV420 = nullptr;
    
    SwsContext* swsCtx = nullptr;
    if (AV_PIX_FMT_YUV420P != dec_ctx->pix_fmt) {
        frame_YUV420 = av_frame_alloc();
        int output_buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, dec_ctx->width, dec_ctx->height, 1);
        if (output_buffer_size < 0) {
            cerr << "av_image_get_buffer_size failed." << output_buffer_size << endl;
            return -1;
        }

        uint8_t* output_buffer = (uint8_t*)av_malloc(output_buffer_size);
        ret = av_image_fill_arrays(frame_YUV420->data, frame_YUV420->linesize, output_buffer, AV_PIX_FMT_YUV420P, dec_ctx->width, dec_ctx->height, 1);
        if (ret < 0) {
            cerr << "av_image_fill_arrays failed." << ret << endl;
            return ret;
        }

        swsCtx = sws_getContext(dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
            dec_ctx->width, dec_ctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    }

    // YUV420 file
    ofstream yuv420("./video.yuv", std::ios::binary);

    // packet 
    AVPacket* pkt = av_packet_alloc();
    av_init_packet(pkt);

    while (av_read_frame(fmt_ctx, pkt) == 0) {
        if (pkt->stream_index != target_stream_idx) {
            continue;
        }

        int got_picutre = 0;
        ret = avcodec_decode_video2(dec_ctx, frame, &got_picutre, pkt);
        if (ret < 0) {
            cerr << "avcodec_decode_video2 -> " << ret << endl;
            continue;
        }

        cout << "Got v-frame " << v_frame_idx++;
        if (got_picutre != 0) {
            uint8_t** data = nullptr;
            if (swsCtx!=nullptr && frame_YUV420!=nullptr) {
                ret = sws_scale(swsCtx, (const unsigned char* const*)frame->data, frame->linesize, 0, dec_ctx->height, frame_YUV420->data, frame_YUV420->linesize);
                data = frame_YUV420->data;
            }
            else {
                data = frame->data;
            }
            
            int Y_size = dec_ctx->width * dec_ctx->height;
            int U_size = Y_size / 4;
            int V_size = U_size;

            yuv420.write((const char *)data[0], Y_size);
            yuv420.write((const char *)data[1], U_size);
            yuv420.write((const char *)data[2], V_size);

            cout << " extracted " << Y_size+U_size+V_size << " bytes\n";
        }

        av_frame_unref(frame);
        av_frame_unref(frame_YUV420);
    }

    av_frame_free(&frame);
    av_frame_free(&frame_YUV420);
    av_packet_free(&pkt);
    avformat_free_context(fmt_ctx);

	return 0;
}
