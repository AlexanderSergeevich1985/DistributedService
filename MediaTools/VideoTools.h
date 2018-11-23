#pragma once
#ifndef TOOLS_H
#define TOOLS_H
#include <QObject>
#include <atomic>

#include "coders.h"
#include "VideoProvider.h"

namespace MediaTools {

class a : public QObject {
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
