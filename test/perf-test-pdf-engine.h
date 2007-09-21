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

class PdfEnginePoppler {
public:
    PdfEnginePoppler();
    ~PdfEnginePoppler();

    const char *fileName(void) const { return _fileName; };

    void setFileName(const char *fileName) {
        assert(!_fileName);
        _fileName = (char*)strdup(fileName);
    }

    bool validPageNo(int pageNo) const {
        if ((pageNo >= 1) && (pageNo <= pageCount()))
            return true;
        return false;
    }

    int pageCount(void) const { return _pageCount; }

    virtual bool load(const char *fileName);
    virtual int pageRotation(int pageNo);
    virtual SizeD pageSize(int pageNo);
    virtual SplashBitmap *renderBitmap(int pageNo, double zoomReal, int rotation,
                         BOOL (*abortCheckCbkA)(void *data),
                         void *abortCheckCbkDataA);

    PDFDoc* pdfDoc() { return _pdfDoc; }
    SplashOutputDev *   outputDevice();
private:
    char *_fileName;
    int _pageCount;

    PDFDoc *            _pdfDoc;
    SplashOutputDev *   _outputDev;
};


#endif
