/**
 * @file    simplewidget.h
 * @brief   
 * @version 00.10
 *
 * Put the file comments here.
 *
 * @verbatim
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2005
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 * @endverbatim
 */

#ifndef _SIMPLEWIDGET_H
#define _SIMPLEWIDGET_H

//#define FONT_FILE "data/fonts/decker.ttf"
#define FONT_FILE "data/fonts/simfang.ttf"

#define BYTES_PER_PIXEL 2           //!< Only 16 bit color supported.

#define SIMPLEWIDGET_SUCCESS 0      //!< Success return code.
#define SIMPLEWIDGET_FAILURE -1     //!< Failure return code.

typedef void * simplewidget_button; //!< Opaque handle to a button
typedef void * simplewidget_screen; //!< Opaque handle to a screen
typedef void * simplewidget_text;   //!< Opaque handle to a line of text
typedef void * simplewidget_jpeg;   //!< Opaque handle to a jpeg image
typedef void * simplewidget_png;    //!< Opaque handle to a png image

typedef struct _simplescreen {
    unsigned short *bufp;
    int w;
    int h;
} _simplescreen;

#define TEXT_MAX_TEXT_LENGTH 100

typedef struct _simpletext {
    int x;                            // The x position of the button.
    int y;                            // The y position of the button.
    int h;                            // The height of the font.
    char txt[TEXT_MAX_TEXT_LENGTH];   // The text to render.
} _simpletext;

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * @brief Initializes a simplewidget screen. Must be called before any
 * other simplewidget API call on this screen.
 * @param fbp The pointer to the framebuffer.
 * @param fbw The width of the framebuffer.
 * @param fbh The height of the framebuffer.
 * @param swsp Screen pointer returned.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */

extern int png_load(char *filename, unsigned char *yuvbuf);
extern int png_load_noalpha(char *filename, unsigned char *yuvbuf);

extern int simplewidget_text_load(char *text,unsigned char *ptxtBuf,int font_width,int font_height);
extern int dotfont_text_load(char *text,unsigned char *ptxtBuf,int font_width,int font_height);

extern int ttffont_constwidth_text_show(simplewidget_text swt,simplewidget_screen sws,int font_width);
extern int dotfont_constwidth_text_show(simplewidget_text swt,simplewidget_screen sws,int font_width);
extern int constwidth_text_getsize(char *text, int font_width, int font_height, int *string_width, int *string_height);

extern int editbox_text_show(simplewidget_text swt,simplewidget_screen sws);
extern int editbox_text_getsize(char *text, int font_height, int *string_width, int *string_height);

extern int simplewidget_png_getsize(char *filename,int *png_width,int *png_height);
extern int simplewidget_jpeg_getsize(char *filename,int *jpeg_width,int *jpeg_height);
extern int simplewidget_text_getsize(char *text, int font_height, int *string_width,
										  int *string_height);

extern int simplewidget_screen_init(void *fbp, int fbw, int fbh,
                                    simplewidget_screen *swsp);

/**
 * @brief Clear an area of the screen.
 * @param sws Simplewidget screen to clear on.
 * @param x X coordinate of area to clear.
 * @param y Y coordinate of area to clear.
 * @param w Width of area to clear.
 * @param h Height of area to clear.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_screen_clear(simplewidget_screen sws,
                                     int x, int y, int w, int h);

/**
 * @brief Draw a rectangle on the screen.
 * @param sws Simplewidget screen to draw rectangle on.
 * @param x X coordinate of the rectangle.
 * @param y Y coordinate of the rectangle.
 * @param w Width of the rectangle.
 * @param h Height of the rectangle.
 * @param r Red component of rectangle color.
 * @param g Green component of rectangle color.
 * @param b Blue component of rectangle color.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
int simplewidget_screen_draw_rectangle(simplewidget_screen sws, int x, int y,
                                       int w, int h, int r, int g, int b);

int simplewidget_screen_draw_line(simplewidget_screen sws, int x1, int y1,
                                       int x2, int y2, int r, int g, int b);

int simplewidget_screen_draw_triangle(simplewidget_screen sws, int x1, int y1,
								  int x2, int y2, int x3, int y3, int r, int g, int b);

int simplewidget_screen_draw_loop(simplewidget_screen sws, int x, int y,
                                       int w, int h, int r, int g, int b);

int simplewidget_erase_rectangle(int x,int y,int w,int h, simplewidget_screen sws);
/**
 * @brief Deinitalize the simplewidget screen and free up associated resources.
 * @param sws Simplewidget screen to deinitalize.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_screen_exit(simplewidget_screen sws);

/**
 * @brief Creates a button, but does not show it on screen.
 * @param x X position of the button on screen.
 * @param y Y position of the button on screen.
 * @param w Width of button.
 * @param h Height of button.
 * @param r Red component of button color.
 * @param g Green component of button color.
 * @param b Blue component of button color.
 * @param font_height Height of the button font.
 * @param border Size of button border.
 * @param txt Text to render inside button.
 * @param swbp Simplewidget handle returned.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_button_create(int x, int y, int w, int h,
                                      int r, int g, int b, int font_height,
                                      int border, char *txt,
                                      simplewidget_button *swbp);

/**
 * @brief Show the button on the screen.
 * @param swb Simplewidget handle of the button to show.
 * @param sws Simplewidget screen to show button on.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_button_show(simplewidget_button swb,
                                    simplewidget_screen sws);

/**
 * @brief Hide a button (un-show it).
 * @param swb Simplewidget handle of the button to hide.
 * @param sws Simplewidget screen to hide button on.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_button_hide(simplewidget_button swb,
                                    simplewidget_screen sws);

/**
 * @brief Press the button.
 * @param swb Simplewidget handle of the button to press.
 * @param sws Simplewidget screen to fade in button on.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_button_press(simplewidget_button swb,
                                     simplewidget_screen sws);

/**
 * @brief Release the button from being pressed.
 * @param swb Simplewidget handle of the button to release from being pressed.
 * @param sws Simplewidget screen to fade in button on.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_button_release(simplewidget_button swb,
                                       simplewidget_screen sws);

/**
 * @brief Delete the button and resources associated with it.
 * @param swb Simplewidget handle of the button to delete.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_button_delete(simplewidget_button swb);

/**
 * @brief Creates a line of text.
 * @param x Starting X position of the line of text.
 * @param y Starting Y position of the line of text.
 * @param height Height of font to render text with.
 * @param txt Text to render inside button.
 * @param swtp Simplewidget handle returned.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_text_create(int x, int y, int height, char *txt,
                                    simplewidget_text *swtp);

/**
 * @brief Show the line of text on the screen.
 * @param swt Simplewidget handle of the text line to show.
 * @param sws Simplewidget screen to show text line on.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_text_show(simplewidget_text swt,
                                  simplewidget_screen sws);

/**
 * @brief Delete the line of text and resources associated with it.
 * @param swt Simplewidget handle of the text line to delete.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_text_delete(simplewidget_text swt);

/**
 * @brief Creates a jpeg image in memory from a file.
 * @param filename The full path and filename of the jpeg file.
 * @param swjp Simplewidget handle returned.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_jpeg_create(char *filename, simplewidget_jpeg *swjp);

/**
 * @brief Shows a previously created jpeg image on screen.
 * @param swj Simplewidget handle to the jpeg image previously created.
 * @param sws Simplewidget screen to show jpeg image on.
 * @param x Starting X position on screen of the jpeg image.
 * @param y Starting Y position on screen of the jpeg image.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_jpeg_show(char * filename, simplewidget_screen sws, int x, int y);

/**
 * @brief Free up resources associated with a previously created jpeg image.
 * @param swj Simplewidget handle to the jpeg image previously created.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_jpeg_delete(simplewidget_jpeg swj);

/**
 * @brief Creates a jpeg image in memory from a file.
 * @param filename The full path and filename of the png file.
 * @param swpp Simplewidget handle returned.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_png_create(char *filename, simplewidget_png *swpp);

/**
 * @brief Shows a previously created png image on screen.
 * @param swp Simplewidget handle to the png image previously created.
 * @param sws Simplewidget screen to show png image on.
 * @param x Starting X position on screen of the png image.
 * @param y Starting Y position on screen of the png image.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_png_show(char * filename,simplewidget_screen sws,int x, int y,int flag);

/**
 * @brief Free up resources associated with a previously created png image.
 * @param swp Simplewidget handle to the png image previously created.
 * @return SIMPLEWIDGET_SUCCESS on success and SIMPLEWIDGET_FAILURE on failure.
 */
extern int simplewidget_png_delete(simplewidget_png swp);

extern simplewidget_jpeg g_pic;
#if defined (__cplusplus)
}
#endif

#endif // _SIMPLEWIDGET_H
