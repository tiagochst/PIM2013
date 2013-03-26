#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "Eigen/Dense"

class Image {

 private:
  int m_height;
  int m_width;
  int m_maxGreyLevel;
  Eigen::MatrixXd m_figure;
  
 public:
  Image(std::string fileName);
  virtual ~Image();
  
  void SetHeight     (int newHeight);
  void SetWidth      (int newWidth) ;
  void SetGreyLevel  (int newGreyLevel); 

  int GetHeight      () ;
  int GetWidth       () ;
  int GetMaxGreyLevel();

  /* Read a binary (P5) or Asc(P2) .pgm file*/
  void loadFromFile(std::string fileName);
 
  /* Verify if image was read correctly*/
  void CreateAscPgm(std::string fileName);
  
};

#endif /* IMAGE_H_ */


