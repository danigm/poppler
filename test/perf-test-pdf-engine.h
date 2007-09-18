#ifndef PDF_TEST_PDF_ENGINE_H_
#define PDF_TEST_PDF_ENGINE_H_

#include "ErrorCodes.h"
#include "GooString.h"
#include "GooList.h"
#include "GlobalParams.h"
#include "SplashBitmap.h"
#include "Object.h" /* must be included before SplashOutputDev.h because of sloppiness in SplashOutputDev.h */
#include "SplashOutputDev.h"
#include "TextOutputDev.h"
#include "PDFDoc.h"
#include "SecurityHandler.h"
#include "Link.h"

#define INVALID_PAGE_NO     -1

#ifdef _MSC_VER
#define strdup _strdup
#endif

#define dimof(X)    (sizeof(X)/sizeof((X)[0]))

class SizeD {
public:
    SizeD(double dx, double dy) { m_dx = dx; m_dy = dy; }
    SizeD(int dx, int dy) { m_dx = (double)dx; m_dy = (double)dy; }
    SizeD() { m_dx = 0; m_dy = 0; }
    int dxI() { return (int)m_dx; }
    int dyI() { return (int)m_dy; }
    double dx() { return m_dx; }
    double dy() { return m_dy; }
    void setDx(double dx) { m_dx = dx; }
    void setDy(double dy) { m_dy = dy; }
private:
    double m_dx;
    double m_dy;
};

/* Abstract class representing cached bitmap. Allows different implementations
   on different platforms. */
class RenderedBitmap {
public:
    virtual ~RenderedBitmap() {};
    virtual int dx() = 0;
    virtual int dy() = 0;
    virtual int rowSize() = 0;
    virtual unsigned char *data() = 0;

#ifdef WIN32
    // TODO: this is for WINDOWS only
    virtual HBITMAP createDIBitmap(HDC) = 0;
    virtual void stretchDIBits(HDC, int, int, int, int) = 0;
#endif
};

class RenderedBitmapSplash : public RenderedBitmap {
public:
    RenderedBitmapSplash(SplashBitmap *);
    virtual ~RenderedBitmapSplash();

    virtual int dx();
    virtual int dy();
    virtual int rowSize();
    virtual unsigned char *data();

#ifdef WIN32
    virtual HBITMAP createDIBitmap(HDC);
    virtual void stretchDIBits(HDC, int, int, int, int);
#endif
protected:
    SplashBitmap *_bitmap;
};

class PdfEngine {
public:
    PdfEngine() : 
        _fileName(0)
        , _pageCount(INVALID_PAGE_NO) 
    { }

    virtual ~PdfEngine() { free((void*)_fileName); }

    const char *fileName(void) const { return _fileName; };

    void setFileName(const char *fileName) {
        assert(!_fileName);
        _fileName = (const char*)strdup(fileName);
    }

    bool validPageNo(int pageNo) const {
        if ((pageNo >= 1) && (pageNo <= pageCount()))
            return true;
        return false;
    }

    int pageCount(void) const { return _pageCount; }

    virtual bool load(const char *fileName) = 0;
    virtual int pageRotation(int pageNo) = 0;
    virtual SizeD pageSize(int pageNo) = 0;
    virtual RenderedBitmap *renderBitmap(int pageNo, double zoomReal, int rotation,
                         BOOL (*abortCheckCbkA)(void *data),
                         void *abortCheckCbkDataA) = 0;

protected:
    const char *_fileName;
    int _pageCount;
};

class PdfEnginePoppler : public PdfEngine {
public:
    PdfEnginePoppler();
    virtual ~PdfEnginePoppler();
    virtual bool load(const char *fileName);
    virtual int pageRotation(int pageNo);
    virtual SizeD pageSize(int pageNo);
    virtual RenderedBitmap *renderBitmap(int pageNo, double zoomReal, int rotation,
                         BOOL (*abortCheckCbkA)(void *data),
                         void *abortCheckCbkDataA);

    PDFDoc* pdfDoc() { return _pdfDoc; }
    SplashOutputDev *   outputDevice();
private:

    PDFDoc *            _pdfDoc;
    SplashOutputDev *   _outputDev;
};


#endif
