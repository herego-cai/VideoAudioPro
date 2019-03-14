#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QDateTime>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //1.注册
    av_register_all();

    //2.初始化网络流媒体
    avformat_network_init();

    //3.SDL初始化
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)){
        qDebug()<<"SDL_Init failure";
        return -1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}

////Buffer:
////|-----------|-------------|
////chunk-------pos---len-----|
//static  Uint8  *audio_chunk;
//static  Uint32  audio_len;
//static  Uint8  *audio_pos;

///* Audio Callback
// * The audio function callback takes the following parameters:
// * stream: A pointer to the audio buffer to be filled
// * len: The length (in bytes) of the audio buffer
// *
//*/
//void  fill_audio(void *udata,Uint8 *stream,int len){
//    //SDL 2.0
//    SDL_memset(stream, 0, len);
//    if(audio_len==0)
//            return;
//    len=(len>audio_len?audio_len:len);

//    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
//    audio_pos += len;
//    audio_len -= len;
//}

//int main(int argc, char* argv[])
//{
//    //Init
//    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
//        qDebug( "Could not initialize SDL - %s\n", SDL_GetError());
//        return -1;
//    }
//    //SDL_AudioSpec
//    SDL_AudioSpec wanted_spec;
//    wanted_spec.freq = 32000;
//    wanted_spec.format = AUDIO_S16SYS;
//    wanted_spec.channels = 1;
//    wanted_spec.silence = 0;
//    wanted_spec.samples = 1024;
//    wanted_spec.callback = fill_audio;

//    if (SDL_OpenAudio(&wanted_spec, NULL)<0){
//        qDebug("can't open audio.\n");
//        return -1;
//    }

//    FILE *fp=fopen("D:\\1.pcm","rb+");
//    if(fp==NULL){
//        qDebug("cannot open this file\n");
//        return -1;
//    }
//    int pcm_buffer_size=4096;
//    char *pcm_buffer=(char *)malloc(pcm_buffer_size);
//    int data_count=0;

//    //Play
//    SDL_PauseAudio(0);

//    while(1){
//        if (fread(pcm_buffer, 1, pcm_buffer_size, fp) != pcm_buffer_size){
//            // Loop
//            fseek(fp, 0, SEEK_SET);
//            fread(pcm_buffer, 1, pcm_buffer_size, fp);
//            data_count=0;
//        }
//        qDebug("Now Playing %10d Bytes data.\n",data_count);
//        data_count+=pcm_buffer_size;
//        //Set audio buffer (PCM data)
//        audio_chunk = (Uint8 *) pcm_buffer;
//        //Audio buffer length
//        audio_len =pcm_buffer_size;
//        audio_pos = audio_chunk;

//        while(audio_len>0)//Wait until finish
//            SDL_Delay(1);
//    }
//    free(pcm_buffer);
//    SDL_Quit();

//    return 0;
//}


//#ifdef __cplusplus
//extern "C"
//{
//#include <libavcodec\avcodec.h>
//#include <libavformat\avformat.h>
//#include <libswscale\swscale.h>
//#include <libavutil\avutil.h>
//}
//#endif

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <windows.h>
//#include <excpt.h>
//#include <QDebug>

//#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

//void printAVFormatContext(AVFormatContext *ctx)
//{
//    qDebug()<<"ctx->audio_codec:"<<ctx->audio_codec;
//    qDebug()<<"ctx->audio_codec_id:"<<ctx->audio_codec_id;
//    qDebug()<<"ctx->audio_preload:"<<ctx->audio_preload;
//    qDebug()<<"ctx->avio_flags:"<<ctx->avio_flags;
//    qDebug()<<"ctx->avoid_negative_ts:"<<ctx->avoid_negative_ts;
//    qDebug()<<"ctx->av_class:"<<ctx->av_class;
//    qDebug()<<"ctx->bit_rate:"<<ctx->bit_rate;
//    qDebug()<<"ctx->codec_whitelist:"<<ctx->codec_whitelist;
//    qDebug()<<"ctx->correct_ts_overflow:"<<ctx->correct_ts_overflow;
//    qDebug()<<"ctx->ctx_flags:"<<ctx->ctx_flags;
//    qDebug()<<"ctx->data_codec:"<<ctx->data_codec;
//    qDebug()<<"ctx->data_codec_id:"<<ctx->data_codec_id;
//    qDebug()<<"ctx->duration:"<<ctx->duration;
//    qDebug()<<"ctx->filename:"<<ctx->filename;
//    qDebug()<<"ctx->flags:"<<ctx->flags;
//    qDebug()<<"ctx->flush_packets:"<<ctx->flush_packets;
//    qDebug()<<"ctx->format_probesize:"<<ctx->format_probesize;
//    qDebug()<<"ctx->format_whitelist:"<<ctx->format_whitelist;
//    qDebug()<<"ctx->fps_probe_size:"<<ctx->fps_probe_size;
//    qDebug()<<"ctx->error_recognition:"<<ctx->error_recognition;
//    qDebug()<<"ctx->event_flags:"<<ctx->event_flags;
//    qDebug()<<"ctx->iformat:"<<ctx->iformat;
//    qDebug()<<"ctx->internal:"<<ctx->internal;
//    qDebug()<<"ctx->io_repositioned:"<<ctx->io_repositioned;
//    qDebug()<<"ctx->key:"<<ctx->key;
//    qDebug()<<"ctx->keylen:"<<ctx->keylen;
//    qDebug()<<"ctx->max_analyze_duration:"<<ctx->max_analyze_duration;
//    qDebug()<<"ctx->max_chunk_duration:"<<ctx->max_chunk_duration;
//    qDebug()<<"ctx->max_chunk_size:"<<ctx->max_chunk_size;
//    qDebug()<<"ctx->max_delay:"<<ctx->max_delay;
//    qDebug()<<"ctx->max_index_size:"<<ctx->max_index_size;
//    qDebug()<<"ctx->max_interleave_delta:"<<ctx->max_interleave_delta;
//    qDebug()<<"ctx->max_picture_buffer:"<<ctx->max_picture_buffer;
//    qDebug()<<"ctx->max_ts_probe:"<<ctx->max_ts_probe;
//    qDebug()<<"ctx->metadata:"<<ctx->metadata;
//    qDebug()<<"ctx->metadata_header_padding:"<<ctx->metadata_header_padding;
//    qDebug()<<"ctx->nb_chapters:"<<ctx->nb_chapters;
//    qDebug()<<"ctx->nb_programs:"<<ctx->nb_programs;
//    qDebug()<<"ctx->nb_streams:"<<ctx->nb_streams;
//    qDebug()<<"ctx->video_codec_id:"<<ctx->video_codec_id;
//    qDebug()<<"ctx->video_codec:"<<ctx->video_codec;
//    qDebug()<<"ctx->ts_id:"<<ctx->ts_id;
//    qDebug()<<"ctx->subtitle_codec_id:"<<ctx->subtitle_codec_id;
//    qDebug()<<"ctx->subtitle_codec:"<<ctx->subtitle_codec;
//    qDebug()<<"ctx->strict_std_compliance:"<<ctx->strict_std_compliance;
//    qDebug()<<"ctx->start_time_realtime:"<<ctx->start_time_realtime;
//    qDebug()<<"ctx->start_time:"<<ctx->start_time;
//}

//int main(int argc, char* argv[])
//{
//    qDebug()<<"..................";
//    av_register_all();//注册库中所有可用的文件格式和编码器

//    char input_file_name[] = "3.mkv";

//    AVFormatContext *ic = avformat_alloc_context();
//    if(avformat_open_input(&ic,input_file_name,NULL,NULL)!=0)
//    {
//        qDebug("can't open the file %s\n",input_file_name);
//        exit(1);
//    }

//    //打开输入文件
//    if(avformat_find_stream_info(ic,NULL)<0)
//    {
//        qDebug("can't find suitable codec parameters\n");
//        exit(1);
//    }//取出流信息

//    int i;
//    int videoindex=-1;int audioindex=-1;
//    for(i=0;i<ic->nb_streams;i++){
//        if(ic->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
//            videoindex=i;
//            //qDebug("video\n");
//        }else if(ic->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
//            //qDebug("audio\n");
//            audioindex=i;
//        }
//    }

//    qDebug()<<"video";
////-----------------------------------------------------------------------------------//
//    if(videoindex==-1)
//    {
//        qDebug("can't find video stream\n");
//        exit(1);
//    }//没有找到视频流

//   AVCodecContext *vCodecCtx = ic->streams[videoindex]->codec;//取得视频流编码上下文指针
//   AVCodec *vCodec = avcodec_find_decoder(vCodecCtx->codec_id);
//   if(vCodec==NULL)
//   {
//      qDebug("can't find suitable video decoder\n");
//      exit(1);
//   }//找到合适的视频解码器

//   if(avcodec_open2(vCodecCtx,vCodec,NULL)<0)
//   {
//      qDebug("can't open the video decoder\n");
//      exit(1);
//   }//打开该视频解码器

//   qDebug()<<"audio";
//   //--------------------------------------------------------------------------------//
//   if(audioindex==-1)
//   {
//        qDebug("can't find audio stream\n");
//        exit(1);
//   }//没有找到音频流

//   AVCodecContext *aCodecCtx;
//   aCodecCtx=ic->streams[audioindex]->codec;
//   AVCodec *aCodec;
//   aCodec=avcodec_find_decoder(aCodecCtx->codec_id);
//   if(aCodec==NULL)
//   {
//      qDebug("can't find suitable audio decoder\n");
//      exit(1);
//   }

//   //找到合适的音频解码器
//   if(avcodec_open2(aCodecCtx,aCodec,NULL)<0)
//   {
//      qDebug("can't open the audio decoder\n");
//      exit(1);
//   }//打开该音频解码器
//   //-------------------------------------------------------------------------------------//
//   //下面为输出文件处理部分
//   qDebug()<<"output";
//    char output_file_name[] = "1.mp4";

//    AVFormatContext *oc;
//    AVCodecContext *oVcc,*oAcc;
//    AVCodec *oVc,*oAc;
//    AVStream *video_st,*audio_st;
//    AVFrame *oVFrame,*oAFrame;
//    AVOutputFormat *fmt;

//    double video_pts;//////////////////////////////////////////////////////////////////////////////////////no use

//    oVFrame=av_frame_alloc();
//    oAFrame=av_frame_alloc();
//    fmt=av_guess_format(NULL,output_file_name,NULL);
//    if(!fmt)
//    {
//           qDebug("could not deduce output format from outfile extension\n");
//           exit(0);
//    }//判断是否可以判断输出文件的编码格式

//    qDebug()<<"000000000000000000000000000";
//    oc=avformat_alloc_context();
//    if(!oc)
//    {
//           qDebug("Memory error\n");
//           exit(0);
//    }
//    oc->oformat=fmt;
//    strncpy(oc->filename,output_file_name,sizeof(oc->filename));
////    (oc->priv_data, "preset", "superfast", 0);
////    av_opt_set(oc->priv_data, "tune", "zerolatency", 0);

////////////////////////////////视频
//    qDebug()<<"==============output video============";
//    qDebug()<<"fmt->video_codec:"<<fmt->video_codec;
//    oVc=avcodec_find_encoder(fmt->video_codec);

//    oVcc = avcodec_alloc_context3(oVc);
//    oVcc->codec_id = fmt->video_codec;//会出现卡死现象,没有分配空间
//    oVcc->codec_type=AVMEDIA_TYPE_VIDEO;
//    oVcc->bit_rate=400000;
//    oVcc->width=vCodecCtx->width;
//    oVcc->height=vCodecCtx->height;

//    video_st=avformat_new_stream(oc,oVc);
//    if(!video_st)
//    {
//          qDebug("could not alloc video stream\n");
//          exit(0);
//    }

//    oVcc->time_base.den = ic->streams[videoindex]->r_frame_rate.num;
//    oVcc->time_base.num = ic->streams[videoindex]->r_frame_rate.den;
//    oVcc->gop_size=vCodecCtx->gop_size;
//    oVcc->pix_fmt=vCodecCtx->pix_fmt;
//    oVcc->max_b_frames=vCodecCtx->max_b_frames;
//    //video_st->r_frame_rate=ic->streams[videoindex]->r_frame_rate;
//    oVcc->flags|= CODEC_FLAG_GLOBAL_HEADER;/////////////////////这样一个标志没注明，才不会导致报错没有使用global headers


///////////////////////////////////////音频

//    qDebug()<<"fmt->audio_codec:"<<fmt->audio_codec;
//    oAc=avcodec_find_encoder(fmt->audio_codec);
//    oAcc=avcodec_alloc_context3(oAc);
//    oAcc->codec_id = fmt->audio_codec;
//    oAcc->codec_type =AVMEDIA_TYPE_AUDIO;
////    oAcc->bit_rate = ic->streams[audioindex]->codec->bit_rate;//aCodecCtx->bit_rate;
//    oAcc->sample_fmt = ic->streams[audioindex]->codec->sample_fmt;
//    oAcc->channel_layout = ic->streams[audioindex]->codec->channel_layout;
//    oAcc->sample_rate = ic->streams[audioindex]->codec->sample_rate;//aCodecCtx->sample_rate;
//    oAcc->channels = ic->streams[audioindex]->codec->channels;
//    oAcc->flags|= CODEC_FLAG_GLOBAL_HEADER;/////////////////////这样一个标志没注明，才不会导致报错没有使用global headers


//    audio_st=avformat_new_stream(oc,oAc);
//    if(!audio_st)
//    {
//           qDebug("could not alloc audio stream\n");
//           exit(0);
//    }

////    av_dump_format(oc, 0, oc->filename, 1);

////    oc->bit_rate = 401582;
////    oc->duration = 307400000;
////    oc->iformat = ic->iformat;
////    oc->metadata = ic->metadata;
////    qDebug()<<"--------------ic-----------";
////    printAVFormatContext(ic);
////    qDebug()<<"--------------oc-----------";
////    printAVFormatContext(oc);
////    int ret1 = avformat_write_header(oc,NULL) ;
////    if (ret1 < 0)
////    {
////        qDebug( "Invalid output format parameters %d",ret1);
////        exit(0);
////    }//设置必要的输出参数
////    strcpy(oc->,ic->title);
////    strcpy(oc->author,ic->author);
////    strcpy(oc->copyright,ic->copyright);
////    strcpy(oc->comment,ic->comment);
////    strcpy(oc->album,ic->album);
////    oc->year=ic->year;
////    oc->track=ic->track;
////    strcpy(oc->genre,ic->genre);

////    dump_format(oc,0,output_file_name,1);//列出输出文件的相关流信息

//    qDebug("ic->streams[audioindex]->codec->bit_rate = %d\n",ic->streams[audioindex]->codec->bit_rate);
//    qDebug("aCodecCtx->sample_rate = %d\n",aCodecCtx->sample_rate);
//    qDebug("ic->streams[videoindex]->r_frame_rate.den %d\n",ic->streams[videoindex]->r_frame_rate.den);
//    qDebug("ic->streams[videoindex]->r_frame_rate.num %d\n",ic->streams[videoindex]->r_frame_rate.num);

//    //-------------------------------------------------------------------------------
//    if(avcodec_open2(oVcc,oVc,NULL)<0)
//    {
//           qDebug("can't open the output video codec\n");
//           exit(0);
//    }//打开视频编码器

//    if(avcodec_open2(oAcc,oAc,NULL)<0)
//    {
//           qDebug("can't open the output audio codec");
//           exit(0);
//    }//打开音频编码器

//    if (!(oc->flags & AVFMT_FLAG_NOFILLIN))
//    {

//       if(avio_open2(&oc->pb,output_file_name,AVIO_FLAG_WRITE,NULL,NULL)<0)
//       {
//              qDebug("can't open the output file %s\n",output_file_name);
//              exit(0);
//       }//打开输出文件
//       qDebug()<<"open the output file success";
//    }

//    if(!oc->nb_streams)
//    {
//           qDebug("output file dose not contain any stream\n");
//           exit(0);
//    }//查看输出文件是否含有流信息

//    if(avformat_write_header(oc,NULL)<0)
//    {
//        qDebug("Could not write header for output file\n");
//        exit(1);
//    }

//    AVPacket packet;
//    av_init_packet(&packet);

//    uint8_t *ptr;
//    uint8_t *out_buf=NULL;

//    static short *samples=NULL;
//    static unsigned int samples_size=0;
//    /////////////////////分配音视频输出缓冲区大小/////////////////////
//    uint8_t *video_outbuf,*audio_outbuf;
//    int video_outbuf_size,audio_outbuf_size;
//    video_outbuf_size=400000;
//    video_outbuf= (uint8_t *) malloc(video_outbuf_size);
//    audio_outbuf_size = FF_MIN_BUFFER_SIZE;
//    audio_outbuf = (uint8_t *)av_malloc(audio_outbuf_size);
//    int out_size=AVCODEC_MAX_AUDIO_FRAME_SIZE;
//    uint8_t * inbuf = (uint8_t *)malloc(out_size);////////////////////////////////////////

//    int flag;
//    int frameFinished;
//    int len=0;
//    int frame_index=0,ret=0;

//   int inputSampleSize = aCodecCtx->frame_size * 2 *aCodecCtx->channels;//获取Sample大小
//   int outputSampleSize = oAcc->frame_size * 2 * oAcc->channels;//获取输出Sample大小////////////////////////////////no use
//   int j=0;

//   qDebug()<<"3333333333333333333333";
//    while(av_read_frame(ic,&packet)>=0)//从输入文件中读取一个包
//    {
//       if(packet.stream_index==videoindex)//判断是否为当前视频流中的包
//       {
//          len=avcodec_decode_video2(vCodecCtx,oVFrame,&frameFinished,&packet);//若为视频包，解码该视频包
//            if(len<0)
//            {
//                 qDebug("Error while decoding\n");
//                 exit(0);
//            }

//            if(frameFinished)//判断视频祯是否读完
//            {
//                fflush(stdout);

//                AVPacket pkt;
//                av_init_packet(&pkt);

//                oVFrame->pts=av_rescale(frame_index,AV_TIME_BASE*(int64_t)oVcc->time_base.num,oVcc->time_base.den);

//                int got_packet = 0;
//                out_size = avcodec_encode_video2(oVcc, &pkt, oVFrame,&got_packet);
//                if (out_size >= 0)
//                {
////                    if(oVcc->coded_frame && oVcc->coded_frame->key_frame)
////                        pkt.flags |= PKT_FLAG_KEY;
////                        pkt.flags = packet.flags;
//                    pkt.stream_index= video_st->index;
////                    pkt.data= video_outbuf;
////                    pkt.size= out_size;
//                    //ret=av_write_frame(oc, &pkt);
//                    if((ret=av_interleaved_write_frame(oc,&pkt))!=0)
//                    {
//                     qDebug("Fail to write the video frame #%d.\n",frame_index);
//                    }
//                    frame_index++;
//                }

//            }else{
//                //ret=av_write_frame(oc, &packet);
//                ret=av_interleaved_write_frame(oc,&packet);
//                if(ret!=0)
//                {
//                    qDebug("while write video frame error\n");
//                    exit(0);
//                }
//            }
//       }
//       else if(packet.stream_index==audioindex)
//       {
//           while (len>0)
//           {
//               out_size = 0;
//               if((ret = avcodec_decode_audio4(aCodecCtx,oAFrame,&out_size,&packet))<0)
//               {
//                qDebug("while decode audio failure\n");
//                exit(0);
//               }

//               if (out_size>0)
//               {
//                   AVPacket pkt;
//                   av_init_packet(&pkt);

//                   int got_packet = 0;
//                   pkt.size= avcodec_encode_audio2(oAcc, &pkt, oAFrame,&got_packet);
//                   if(got_packet>=0){
//                       if (oAcc->coded_frame && oAcc->coded_frame->pts != AV_NOPTS_VALUE)
//                           pkt.pts= av_rescale_q(oAcc->coded_frame->pts, oAcc->time_base, audio_st->time_base);
//                       pkt.flags |= AV_PKT_FLAG_KEY;
//                       pkt.stream_index= audio_st->index;

//                       //* write the compressed frame in the media file
//                       if (av_write_frame(oc, &pkt) != 0)
//                       {
//                           qDebug("Error while writing audio frame\n");
//                           break;
//                       }
//                   }
//               }
//           }
//       }

//       // Free the packet that was allocated by av_read_frame
//       av_free_packet(&packet);
//       j++;
//       //qDebug(" 第%d帧 ",j);
//    }
//    av_write_trailer(oc);

//    for(i = 0; i < oc->nb_streams; i++)
//    {
//       av_freep(&oc->streams[i]->codec);
//       av_freep(&oc->streams[i]);
//    }
//    avio_close(oc->pb);
//    av_free(oc);
//    av_free(oVFrame);
//    av_free(out_buf);
//    avcodec_close(vCodecCtx);
//    avcodec_close(aCodecCtx);
//    avformat_close_input(&ic);
//    return 0;
//}


