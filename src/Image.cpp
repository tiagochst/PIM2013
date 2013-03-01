#include <iostream> // cout, cerr
#include <fstream> // ifstream
#include <sstream> // stringstream
#include "Image.h"

Image::Image(std::string fileName) {
  this->loadFromFile(fileName);
}

void Image::loadFromFile(std::string fileName){

  int width = 0, height = 0,greyLevel = 0, i = 0, j = 0;
  int isBinary = 0;
  std::stringstream ss;
  std::string inputLine = "";

  /* path to our images */
  std::string path ="../resources/Project/Images/";
  
  /* Opening pgm file*/
  std::ifstream infile((path+fileName).c_str(), std::ifstream::in | std::ifstream::binary);


  /* First line : version of pgm file*/
  getline(infile,inputLine);
  if(inputLine.compare("P2") != 0 && inputLine.compare("P5") != 0) 
    std::cerr << "Version error" << fileName.c_str() << inputLine << std::endl;
  else
    std::cout << "Version : " << inputLine << std::endl;
  
  if(inputLine.compare("P5") == 0)
    isBinary= 1;

  /* Second line : comment */
  getline(infile,inputLine);
  std::cout << "Comment : " << inputLine << std::endl;
   
  /* Third line : size 
     Forth line : grey level*/
  infile >> height >> width >> greyLevel ;
  this->SetHeight(height);
  this->SetWidth(width);
  this->SetGreyLevel(greyLevel);

  /* resize matrix to receive the image */
  this->m_figure.resize(this->m_height, this->m_width);

  if(isBinary){

    for(i = 0; i < height; i++)
      for (j = 0; j < width; j++)
	this -> m_figure(i,j) = static_cast<int>(infile.get());

  }
  else {
    ss << infile.rdbuf();
 
    for(i = 0; i < height; i++)
      for (j = 0; j < width; j++) 
	ss >> this -> m_figure(i,j);
  }
  
  infile.close();
}

Image::~Image() {
	// TODO Auto-generated destructor stub
}

void Image::CreateAscPgm(std::string fileName){

  int i = 0, j = 0;
  std::string prefix = "PROG";
  std::ofstream ostr( (prefix + fileName).c_str());

  /* writing header */
  ostr << "P2 " << std::endl;
  ostr << "# CREATOR: PIM380 PROJ" << std::endl;;
  ostr << this -> m_height << " "<< this -> m_width << std::endl; 
  ostr << this -> m_maxGreyLevel << std::endl;
  
  for(i = 0; i < this -> m_height; i++) 
    for(j = 0; j < this -> m_width; j++) 
      ostr <<  this -> m_figure(i,j) << std::endl;

  ostr.close();

}


void Image::SetHeight(int newHeight){
  this -> m_height = newHeight;
}

void Image::SetWidth (int newWidth){ 
  this -> m_width  =  newWidth;
}

void Image::SetGreyLevel (int newGreyLevel){
  this -> m_maxGreyLevel  =  newGreyLevel;
}

int Image::GetHeight(){ 
  return m_height;
}

int Image::GetWidth(){
    return m_width;
}

int Image::GetMaxGreyLevel(){ 
  return m_maxGreyLevel;
}
