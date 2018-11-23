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
#pragma once
#ifndef TOOLS_H
#define TOOLS_H
#include <QObject>
#include <atomic>

#include "coders.h"
#include "VideoProvider.h"

namespace MediaTools {

class VideoTools : public QObject {
    Q_OBJECT
public:
    VideoTools(QObject* parent = 0);
    void codeFrames();
    void takeFrame(QByteArray& barr);
public slots:
    void process();
    void frameUnload(QByteArray& barr) {
        while(!this->pause) {
            usleep(50);
        }
    }
signals:
    void getFrame();
protected:
    void setSize(const int& dst_width_, const int& dst_height_);
    void resetSize(const int& dst_width_, const int& dst_height_);
    void Serialize(const AVPacket& pkt, QByteArray& bufPkt);
private:
    AVFrame* frameOut;
    AVFrame* pFrameRGB;
    AVFrame* frame;
    AVOutputFormat* avOutputFormat;
    AVFormatContext* outctx;
    AVCodec* vcodec;
    AVStream* vstrm;
    SwsContext* swsctx;

    VideoProvider<cv::Mat, QPoint>* videoSource;

    int dst_width = 1366;
    int dst_height = 768;
    const AVRational dst_fps = {30, 1};
    int ret = 0;
    int64_t frame_pts = 0;
    bool end_of_stream = false;
    int got_pkt = 0;
    int cnum = 0;

    std::atomic<bool> pause;

    QByteArray temp_buff;
};

}
#endif // TOOLS_H
