#include <omp.h>
#include <iostream> // cout, cerr
#include <fstream> // ifstream
#include <sstream> // stringstream

#define _USE_MATH_DEFINES
#include <cmath>
#include "Image.h"
#include "Rectangle.h"
#include "SubImage.h"
#include "MathUtils.h"
#include "Tools.h"

#include "Config.h"
#include <iomanip>

Image::Image ()
    :   m_height ( 1 ), 
        m_width ( 1 ), 
        m_maxGreyLevel ( 1 ),
        m_figure ( 0x0  ),
        m_normalisedFigure ( 0x0 )
{
    m_figure = new Eigen::MatrixXi ();
    m_normalisedFigure = new Eigen::MatrixXf ();
    ResetMatrix ();
}
Image::Image (
    const int& iWidth,
    const int& iHeight,
    const int& iGreyLevel
)   :   m_height ( iHeight ), 
        m_width ( iWidth ), 
        m_maxGreyLevel ( iGreyLevel ),
        m_figure ( 0x0  ),
        m_normalisedFigure ( 0x0 )
{
    m_figure = new Eigen::MatrixXi ();
    m_normalisedFigure = new Eigen::MatrixXf ();
    ResetMatrix ();
}
Image::Image (
    const std::string& iFilename
)   :   m_height ( 1 ), 
        m_width ( 1 ), 
        m_maxGreyLevel ( 1 ),
        m_figure ( 0x0  ),
        m_normalisedFigure ( 0x0 )
{
    LoadFromFile ( iFilename );
}
Image::~Image () {
    ClearMatrix ();
}

void Image::loadMat (
   const std::string& iFilename
){
    m_cvImage = imread(iFilename, CV_LOAD_IMAGE_COLOR);   // Read the file

    if(! m_cvImage.data )       // Check for invalid input
    {
      std::cout <<  "Could not open or find the image" << std::endl ;
        return ;
    }

    return;
}


void Image::MatchingMethod(Mat templ,Mat result, Point* maxLoc, double* maxVal)
{

  /// Create the result matrix
  int result_cols =  m_cvImage.cols - templ.cols + 1;
  int result_rows = m_cvImage.rows - templ.rows + 1;

  result.create( result_cols, result_rows, CV_32FC1 );

  /// Do the Matching
  matchTemplate( m_cvImage, templ, result, CV_TM_CCORR_NORMED );
  
  /// Localizing the best match with minMaxLoc
  double minVal; Point minLoc;
  minMaxLoc( result, &minVal, maxVal, &minLoc, maxLoc, Mat() );

  // namedWindow( "Result", CV_WINDOW_AUTOSIZE );
  // imshow( "Result", result );

  // waitKey(0);
 vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(100);
    //    result.convertTo(result,CV_16UC3,255,255);
    cv::normalize(result, result, 0, 255, NORM_MINMAX, CV_8UC1);
    try {
      imwrite(Config::OutputPath()+"testeOpencv.jpeg" , result, compression_params);

    }
    catch (std::runtime_error& ex) {
      fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
      return ;
    }
    
    fprintf(stdout, "Saved PNG file with alpha data.\n");
    
    return;
    
}

void Image::MatchingMethod(Mat templ, double* maxVal)
{

  Mat result;
  const char* image_window = "Source Image";
  const char* result_window = "Result window";

  /// Create the result matrix
  int result_cols =  m_cvImage.cols - templ.cols + 1;
  int result_rows = m_cvImage.rows - templ.rows + 1;

  result.create( result_cols, result_rows, CV_32FC1 );

  /// Do the Matching
  matchTemplate( m_cvImage, templ, result, CV_TM_CCORR_NORMED );
  
  /// Localizing the best match with minMaxLoc
  double minVal; Point minLoc, maxLoc;
  minMaxLoc( result, &minVal, maxVal, &minLoc, &maxLoc, Mat() );

  return;
}

void Image::MatchingMethod(Mat templ, Mat img, double* maxVal)
{

  Mat result;
  const char* image_window = "Source Image";
  const char* result_window = "Result window";

  /// Source image to display
  Mat img_display;
  img.copyTo( img_display );

  /// Create the result matrix
  int result_cols =  img.cols - templ.cols + 1;
  int result_rows = img.rows - templ.rows + 1;

  result.create( result_cols, result_rows, CV_32FC1 );

  /// Do the Matching
  matchTemplate( img, templ, result, CV_TM_CCORR_NORMED );
  
  /// Localizing the best match with minMaxLoc
  double minVal; Point minLoc, maxLoc;
  minMaxLoc( result, &minVal, maxVal, &minLoc, &maxLoc, Mat() );

  return;
}

void Image::CVErrorScore (
   Mat iImageA, double* score
) {

    const unsigned int& SAMPLING_STEP   = 20;
    const int&          NH_SZ           = 9;


    const unsigned int& height  = iImageA.rows;
    const unsigned int& width   = iImageA.cols;

    cv::Mat croppedImage;
    cv::Mat croppedTempl;
    

    for ( int y = NH_SZ / 2; y < height - NH_SZ / 2; y += SAMPLING_STEP ) {
        for ( int x = NH_SZ / 2; x < width - NH_SZ / 2; x += SAMPLING_STEP ) {
            double localScore = 0.f;

	    cv::Rect window(x-NH_SZ / 2, y-NH_SZ / 2, NH_SZ, NH_SZ);
	    cv::Mat(iImageA, window).copyTo(croppedTempl);
	    cv::Mat(m_cvImage, window).copyTo(croppedImage);

	    this -> MatchingMethod(croppedImage,croppedTempl,&localScore); 
	    *score += (1.0 - localScore); 
	}
    }

    return;

}
void Image::LoadFromFile (
    const std::string& iFilename
) {
    int width = 0, height = 0, greyLevel = 0, i = 0, j = 0;
    int isBinary = 0;
    std::stringstream ss;
    std::string inputLine = "";

    /* Opening pgm file*/
    std::ifstream inFile (
        iFilename.c_str (), 
        std::ifstream::in | std::ifstream::binary
    );

    if (inFile.is_open() && inFile.good()) {
        /* First line : version of pgm file*/
        getline( inFile, inputLine );

        if (
                inputLine.compare( "P2" ) != 0 
            &&  inputLine.compare( "P5" ) != 0
        ) {
            std::cerr   << "Version error " 
                        << iFilename.c_str()
                        << " Version: "
                        << inputLine 
                        << std::endl;

        } 
        if ( inputLine.compare( "P5" ) == 0 ) {
            isBinary = 1;
        }

        ClearMatrix ();
        m_figure = new Eigen::MatrixXi ();
        m_normalisedFigure = new Eigen::MatrixXf ();

        /* Second line : comment */
        getline( inFile, inputLine );

        /* Third  line : size
           Fourth line : grey level*/
        inFile >> width >> height >> greyLevel ;

        //SetHeight( height );
        //SetWidth( width );
        SetDimensions ( width, height );
        SetMaxGreyLevel ( greyLevel );

        if ( isBinary ) {
            for ( i = 0; i < height; i++ ) {
                for ( j = 0; j < width; j++ ) {
                    int readValue = static_cast<int>( inFile.get() );

                    SetGreyLvl ( i, j, readValue );
                }
            }
        } else {
            ss << inFile.rdbuf();

            for ( i = 0; i < height; i++ ) {
                for ( j = 0; j < width; j++ ) {
                    int readValue = 0;
                    ss >> readValue;

                    SetGreyLvl ( i, j, readValue );
                }
            }
        }
        inFile.close();
    } else {
        std::cout << iFilename << std::endl;
        std::cerr << ": File could not be opened" << std::endl; 
    }
}


void Image::RecalculateGreyLvl()
{
    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            (*m_figure) ( i, j ) = (*m_normalisedFigure) ( i, j ) * m_maxGreyLevel;
        }
    }
}
void Image::RecalculateNormalised()
{
    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            (*m_normalisedFigure) ( i, j ) = (float)(*m_figure) ( i, j ) / (float)m_maxGreyLevel;
        }
    }
}

inline void Image::ClearMatrix ()
{
    if ( m_figure ) {
        delete m_figure;
        m_figure = 0x0;
    }
    if ( m_normalisedFigure ) {
        delete m_normalisedFigure;
        m_normalisedFigure = 0x0;
    }
}

inline void Image::ResetMatrix ()
{
    m_figure->resize ( m_height, m_width );
    m_normalisedFigure->resize ( m_height, m_width );
}

inline void Image::SetDimensions (
    const int& iWidth,
    const int& iHeight
) {
    m_width = iWidth;
    m_height = iHeight;
    ResetMatrix ();
}

inline void Image::SetHeight (
    const int& iHeight
) {
    m_height = iHeight;
    ResetMatrix ();
}

inline void Image::SetWidth (
    const int& iWidth
) {
    m_width = iWidth;
    ResetMatrix ();
}

inline void Image::SetMaxGreyLevel (
    const int&  iGreyLevel,
    const bool& iRenormalise
) {
    m_maxGreyLevel = iGreyLevel;
    if ( iRenormalise ) {
        RecalculateNormalised ();
    }
}

inline int const& Image::GetHeight ()
const {
    return m_height;
}

inline int const& Image::GetWidth ()
const {
    return m_width;
}

inline int const& Image::GetMaxGreyLevel ()
const {
    return m_maxGreyLevel;
}

inline void Image::SetGreyLvl (
    const int& iRow,
    const int& iCol,
    const int& iValue
) {
    if (
            InRange ( iCol, 0, m_width  - 1 )
        &&  InRange ( iRow, 0, m_height - 1 )
    ) {
        (*m_figure) ( iRow, iCol ) = iValue;
        (*m_normalisedFigure) ( iRow, iCol ) = (float)iValue / (float)m_maxGreyLevel;
    } else {
        throw BadIndex( iCol, iRow );
    }
}

inline void Image::SetNormed (
    const int&      iRow,
    const int&      iCol,
    const float&    iValue
) {
    if (
            InRange ( iCol, 0, m_width  - 1 )
        &&  InRange ( iRow, 0, m_height - 1 )
    ) {
        (*m_figure) ( iRow, iCol ) = iValue * m_maxGreyLevel;
        (*m_normalisedFigure) ( iRow, iCol ) = iValue;
    } else {
        throw BadIndex ( iCol, iRow );
    }
}

inline void Image::SetGreyLvl (
    const CartesianCoordinate&  iPos,
    const int&                  iValue
) {
    SetGreyLvl ( iPos.y, iPos.x, iValue );
}

inline void Image::SetNormed (
    const CartesianCoordinate&  iPos,
    const float&                iValue
) {
    SetNormed ( iPos.y, iPos.x, iValue );
}

inline const int& Image::GetGreyLvl (
    const int& iRow,
    const int& iCol
) const {
    int row = abs(iRow);
    int col = abs(iCol);

    row = IsOdd (row / m_height) ? (row % m_height) : (m_height - (row % m_height) - 1);
    col = IsOdd (col / m_width ) ? (col % m_width ) : (m_width  - (col % m_width ) - 1);
    
    return (*m_figure) ( row, col );
}

inline const int& Image::GetGreyLvl (
    const CartesianCoordinate& iPos
) const {
    return GetGreyLvl ( iPos.y, iPos.x );
}

inline const float& Image::GetNormed (
    const int& iRow,
    const int& iCol
) const {
    int row = abs(iRow);
    int col = abs(iCol);
    
    row = IsOdd(row / m_height) ? (row % m_height) : (m_height - (row % m_height) - 1);
    col = IsOdd(col / m_width ) ? (col % m_width ) : (m_width  - (col % m_width ) - 1);

    return (*m_normalisedFigure) ( row, col );
}

inline const float& Image::GetNormed (
    const CartesianCoordinate& iPos
) const {
    return GetNormed ( iPos.y, iPos.x );
}

void Image::CalculateAnchors (
    const unsigned int&         iTotalFrameCount,
    const unsigned int&         iReferenceFrame,
    const std::string&          iSearchPath,
    const std::string&          iImagePrefix,
    std::vector<unsigned int>&  oAnchorList
) {
    std::string localImagePrefix = iSearchPath + iImagePrefix;

    Image   refImage ( localImagePrefix + toString(iReferenceFrame) + ".pgm");
    Image*  curImage = (Image*)0x0;

    omp_lock_t curImgLock;
    omp_init_lock ( &curImgLock );

    unsigned int curImgIdx  = 0;
    #pragma omp parallel sections shared ( curImage ) 
    {
        #pragma omp section
        {
            while (
                curImgIdx < iTotalFrameCount
            ) {
                if (
                    curImgIdx == iReferenceFrame
                ) {
                    omp_set_lock ( &curImgLock );

                    curImgIdx++;

                    omp_unset_lock ( &curImgLock );

                    continue;
                }

                Image* newImage = new Image ( localImagePrefix + toString(curImgIdx) + ".pgm");

                omp_set_lock ( &curImgLock );

                curImage = newImage;
                curImgIdx++;

                omp_unset_lock ( &curImgLock );
            }
        }

        #pragma omp section
        {
            while (
                curImgIdx <= iTotalFrameCount
            ) {
                omp_set_lock ( &curImgLock );

                if (
                    curImage != (Image*)0x0
                ) {
                    const float score = ImageBase::CalculateErrorScore ( refImage, *curImage );
                    if (
                        score <= 1
                    ) {
                        oAnchorList.push_back ( curImgIdx - 1 );
                    }

                    delete curImage;
                    curImage = (Image*)0x0;
                }

                omp_unset_lock ( &curImgLock );
                
                if (
                    curImgIdx == iTotalFrameCount
                ) {
                    break;
                }
            }
        }
    }
    omp_destroy_lock ( &curImgLock );
}
