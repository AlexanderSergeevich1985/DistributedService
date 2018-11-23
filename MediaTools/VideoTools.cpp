/**The MIT License (MIT)
Copyright (c) 2018 by AleksanderSergeevich
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "VideoTools.h"
#include <unistd.h>
#include <opencv2/opencv.hpp>

namespace MediaTools {

VideoTools::VideoTools(QObject* parent, const char* outfile) : QObject(parent) {
    this->pause = false;
    videoSource = new VideoProvider();
    cv::Mat image;
    videoSource->getNextImage(image);
    dst_width = videoSource.getCols();
    dst_height = videoSource.getRows();
    av_register_all();
    avOutputFormat = av_guess_format(NULL, outfile, NULL);
    if(!avOutputFormat) {
        qDebug() << "Could not deduce output format from file extension: using MPEG.\n";
        avOutputFormat = av_guess_format("mp4", NULL, NULL);
    }
    if(!avOutputFormat) {
        qDebug() << "Could not find suitable output format\n";
        exit(1);
    }
    outctx = NULL;
    outctx = avformat_alloc_context();
    if(!outctx) {
        qDebug() << "Memory error\n";
        exit(1);
    }
    outctx->oformat = avOutputFormat;
    if(ret < 0) {
        return;
    }
    vcodec = avcodec_find_encoder(outctx->oformat->video_codec);
    vstrm = avformat_new_stream(outctx, vcodec);
    if (!vstrm) {
        qDebug() << "fail to execute avformat_new_stream";
        return;
    }
    avcodec_get_context_defaults3(vstrm->codec, vcodec);
    vstrm->codec->width = dst_width;
    vstrm->codec->height = dst_height;
    vstrm->codec->pix_fmt = vcodec->pix_fmts[0];
    vstrm->codec->time_base = vstrm->time_base = av_inv_q(dst_fps);
    vstrm->r_frame_rate = vstrm->avg_frame_rate = dst_fps;
    ret = avcodec_open2(vstrm->codec, vcodec, NULL);
    if(ret < 0) {
        return;
    }
    swsctx = sws_getCachedContext(
                NULL, dst_width, dst_height, AV_PIX_FMT_BGR24,
                dst_width, dst_height, vstrm->codec->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
    if(!swsctx) {
        qDebug() << "fail to execute sws_getCachedContext";
        return;
    }
    frame = avcodec_alloc_frame();
    std::vector<uint8_t> framebuf(avpicture_get_size(vstrm->codec->pix_fmt, dst_width, dst_height));
    avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), vstrm->codec->pix_fmt, dst_width, dst_height);
    frame->width = dst_width;
    frame->height = dst_height;
    frame->format = static_cast<int>(vstrm->codec->pix_fmt);

    unsigned nb_frames = 0;

    QByteArray barr;
}

void VideoTools::process() {
    codeFrames();
}

void VideoTools::setSize(const int& dst_width_, const int& dst_height_) {
    this->dst_width = dst_width_;
    this->dst_height = dst_height_;
}

void VideoTools::resetSize(const int& dst_width_, const int& dst_height_) {
    this->setSize(dst_width_, dst_height_);
    int numBytes = avpicture_get_size(AV_PIX_FMT_BGR24, dst_width, dst_height) ; //AV_PIX_FMT_RGB24
    std::vector<uint8_t> buffer(numBytes);
    avpicture_fill((AVPicture *)pFrameRGB, buffer.data(), AV_PIX_FMT_BGR24, dst_width, dst_height);
    avpicture_fill((AVPicture *)frameOut, buffer.data(), AV_PIX_FMT_BGR24, dst_width, dst_height);
}

void VideoTools::takeFrame(QByteArray& barr) {
    while(!this->pause) {
        usleep(50);
    }
    barr = temp_buff;
    this->pause = false;
}

void VideoTools::codeFrames() {
    // allocate frame buffer for encoding
    frame = avcodec_alloc_frame();
    std::vector<uint8_t> framebuf(avpicture_get_size(vstrm->codec->pix_fmt, dst_width, dst_height));
    avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), vstrm->codec->pix_fmt, dst_width, dst_height);
    frame->width = dst_width;
    frame->height = dst_height;
    frame->format = static_cast<int>(vstrm->codec->pix_fmt);
    do {
        while(this->pause) {
            if(!frame) {
                frame = avcodec_alloc_frame();
                std::vector<uint8_t> framebuf(avpicture_get_size(vstrm->codec->pix_fmt, dst_width, dst_height));
                avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), vstrm->codec->pix_fmt, dst_width, dst_height);
                frame->width = dst_width;
                frame->height = dst_height;
                frame->format = static_cast<int>(vstrm->codec->pix_fmt);
            }
            usleep(100);
        }
        temp_buff.clear();
        cv::Mat image;
        if (!end_of_stream) {
            videoSource->getNextImage(image);
            cv::Size size(640, 480);
            cv::resize(image,image,size);
            const int stride[] = { static_cast<int>(image.step[0]) };
            sws_scale(swsctx, &image.data, stride, 0, image.rows, frame->data, frame->linesize);
            frame->pts = frame_pts++;
        }
        AVPacket pkt0;
        pkt0.data = NULL;
        pkt0.size = 0;
        av_init_packet(&pkt0);
        ret = avcodec_encode_video2(vstrm->codec, &pkt0, end_of_stream ? NULL : frame, &got_pkt);
        if (ret < 0) {
            break;
        }
        Serialize(pkt0, temp_buff);
        av_free_packet(&pkt0);
        this->pause = true;
    } while ((!end_of_stream || got_pkt) && cnum < 700000);
}

void VideoTools::Serialize(const AVPacket& pkt, QByteArray& bufPkt) {
    QHash<QString,QVariant> options;
    options["dts"] = (qint64)pkt.dts;
    options["pts"] = (qint64)pkt.pts;
    options["duration"] = pkt.duration;
    options["position"] = (qint64)pkt.pos;
    options["flag"] = pkt.flags;
    options["size"] = pkt.size;
    options["data"] = QByteArray::fromRawData((const char*)pkt.data, pkt.size);
    QDataStream bds(&bufPkt, QIODevice::WriteOnly);
    bds << options;
}

}
