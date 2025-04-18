#ifndef __FACE_H__
#define __FACE_H__

#define WGET_CMD "wget http://127.0.0.1:8080/?action=snapshot -O /tmp/SearchFace.jpg"
#define PICTURE_PATH "/tmp/SearchFace.jpg"

extern void face_Init(void);
extern void face_Final(void);
extern double face_identification(void);

#endif