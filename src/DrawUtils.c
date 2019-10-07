/***********************************************************************
 Utilities for loading and drawing sprites.
*/
#include<GL/glew.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

/* Load a file into an OpenGL texture, and return that texture. */
GLuint glTexImageTGAFile(const char* filename, int* outWidth, int* outHeight)
{
    const int BPP = 4;
    
    /* open the file */
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "File: %s -- Could not open for reading.\n", filename);
        return 0;
    }

    /* skip first two bytes of data we don't need */
    fseek(file, 2, SEEK_CUR);

    /* read in the image type.  For our purposes the image type should
     * be either a 2 or a 3. */
    unsigned char imageTypeCode;
    fread(&imageTypeCode, 1, 1, file);
    if (imageTypeCode != 2 && imageTypeCode != 3) {
        fclose(file);
        fprintf(stderr, "File: %s -- Unsupported TGA type: %d\n", filename, imageTypeCode);
        return 0;
    }

    /* skip 9 bytes of data we don't need */
    fseek(file, 9, SEEK_CUR);

    /* read image dimensions */
    int imageWidth = 0;
    int imageHeight = 0;
    int bitCount = 0;
    fread(&imageWidth, sizeof(short), 1, file);
    fread(&imageHeight, sizeof(short), 1, file);
    fread(&bitCount, sizeof(unsigned char), 1, file);
    fseek(file, 1, SEEK_CUR);

    /* allocate memory for image data and read it in */
    unsigned char* bytes = (unsigned char*)calloc(imageWidth * imageHeight * BPP, 1);

    /* read in data */
    if (bitCount == 32) {
        int it;
        for (it = 0; it != imageWidth * imageHeight; ++it) {
            bytes[it * BPP + 0] = fgetc(file);
            bytes[it * BPP + 1] = fgetc(file);
            bytes[it * BPP + 2] = fgetc(file);
            bytes[it * BPP + 3] = fgetc(file);
        }
    } else {
        int it;
        for (it = 0; it != imageWidth * imageHeight; ++it) {
	    bytes[it * BPP + 0] = fgetc(file);
	    bytes[it * BPP + 1] = fgetc(file);
	    bytes[it * BPP + 2] = fgetc(file);
	    bytes[it * BPP + 3] = 255;
        }
    }

    fclose(file);

    /* load into OpenGL */
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, bytes);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    free(bytes);

    if (outWidth) {
        *outWidth = imageWidth;
    }
    if (outHeight) {
        *outHeight = imageHeight;
    }
    return tex;
}

/* Draw the sprite */
void glDrawSprite(GLuint tex, int x, int y, int w, int h)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    {
	glColor3ub(255, 255, 255);
	glTexCoord2f(0, 1);
	glVertex2i(x, y);
	glTexCoord2f(1, 1);
	glVertex2i(x + w, y);
	glTexCoord2f(1, 0);
	glVertex2i(x + w, y + h);
	glTexCoord2f(0, 0);
	glVertex2i(x, y + h);
    }
    glEnd();
}
