# vPlay
A simple media player
# development environment
1.qt creator 4.10.2  
2.qt 5.9.7 build static  
3.ffmpeg git-2020-01-13-7225479 https://ia801705.us.archive.org/10/items/zeranoe/win32  
4.dx11 DXSDK_Feb10.exe  
5.sdl2 https://github.com/libsdl-org/SDL  
6.libass https://github.com/libass/libass  
# preview
![image](https://user-images.githubusercontent.com/18226546/160064120-a8408e7a-6788-43b3-ac7b-55f6f91ac308.png)
![image](https://user-images.githubusercontent.com/18226546/160064177-06bd9e22-b86a-4500-9324-e630983222fd.png)
![image](https://user-images.githubusercontent.com/18226546/160063659-37695525-2895-4ef1-84e0-cb548e5f5204.png)
![image](https://user-images.githubusercontent.com/18226546/160063682-a3d83c7f-3094-4d4b-a54e-9337ae465231.png)
![image](https://user-images.githubusercontent.com/18226546/160063980-13f2c144-eb52-4b3b-a76b-90ea2f561967.png)
# note
## 1.remove dx11
DEFINES += RENDER_DX11-->//DEFINES += RENDER_DX11
## 2.remove thumbnail
DEFINES += THUMBNAIL-->//DEFINES += THUMBNAIL
## 3.remove ass
DEFINES += AAS_RENDER-->//DEFINES += AAS_RENDER
## 4.remove audio wave display
DEFINES += AUDIO_WAVE_DISPLAY-->//DEFINES += AUDIO_WAVE_DISPLAY
## 5.remove audio filter
#define AUDIO_FILTER-->//#define AUDIO_FILTER
## 6.remove video filter
#define VIDEO_FILTER-->//#define VIDEO_FILTER
