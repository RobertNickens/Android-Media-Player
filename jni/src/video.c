#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#include "SDL.h"
#include "SDL_thread.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
  AVFormatContext *pFormatCtx;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx;
  AVCodec         *pCodec;
  AVFrame         *pFrame; 
  AVPacket        packet;
  int             frameFinished;
  float           aspect_ratio;
  static struct SwsContext *img_convert_ctx;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* txt;
void** pixels;
int txtLocked;
int* pitch;
SDL_Rect        rect;
//SDL_Event       event;


  if(argc < 2) {
    fprintf(stderr, "Usage: test <file>\n");
    exit(1);
  }
  // Register all formats and codecs
  av_register_all();
  
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    exit(1);
  }

  // Open video file
  if(av_open_input_file(&pFormatCtx, argv[1], NULL, 0, NULL)!=0)
    return -1; // Couldn't open file
  
  // Retrieve stream information
  if(av_find_stream_info(pFormatCtx)<0)
    return -1; // Couldn't find stream information
  
  // Dump information about file onto standard error
  dump_format(pFormatCtx, 0, argv[1], 0);
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream
  
  // Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  
  // Open codec
  if(avcodec_open(pCodecCtx, pCodec)<0)
    return -1; // Could not open codec
  
  // Allocate video frame
  pFrame=avcodec_alloc_frame();

//@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Create the window where we will draw.
        window = SDL_CreateWindow("Texture - Window",
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        512, 512,
                        SDL_WINDOW_SHOWN);

        // We must call SDL_CreateRenderer in order for draw calls to affect this window.
        renderer = SDL_CreateRenderer(window, -1, 0);
txt = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, 128, 128);

  // Read frames and save first five frames to disk
  i=0;
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
			   &packet);
      
      // Did we get a video frame?

      if(frameFinished) {
//int
SDL_LockTexture(txt, NULL, pixels, pitch);

//int
//SDL_QueryTexturePixels(txt, pixels, pitch);

	AVPicture pict;
/*
	pict.data[0] = bmp->pixels[0];
	pict.data[1] = bmp->pixels[2];
	pict.data[2] = bmp->pixels[1];

	pict.linesize[0] = bmp->pitches[0];
	pict.linesize[1] = bmp->pitches[2];
	pict.linesize[2] = bmp->pitches[1];
*/

	// Convert the image into YUV format that SDL uses
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
 pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
 PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize,
 0, pCodecCtx->height, pict.data, pict.linesize);
	
SDL_UnlockTexture(txt);
	
	rect.x = 0;
	rect.y = 0;
	rect.w = pCodecCtx->width;
	rect.h = pCodecCtx->height;
//int
SDL_RenderCopy(renderer, txt, NULL, NULL);

      
      }

    }

    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
/*
    SDL_PollEvent(&event);
    switch(event.type) {
    case SDL_QUIT:
      SDL_Quit();
      exit(0);
      break;
    default:
      break;
    }
*/
  }
  
  // Free the YUV frame
  av_free(pFrame);
  
  // Close the codec
  avcodec_close(pCodecCtx);
  
  // Close the video file
  av_close_input_file(pFormatCtx);
  
  return 0;
}
