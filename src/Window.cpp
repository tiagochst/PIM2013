#include <GL/glew.h>
#include "ParameterHandler.h"
#include "Window.h"
#include <sstream>
#include <QGraphicsScene>
#include <QImage>

#include "Image.h"
#include "PPMImage.h"
#include "Frame.h"
#include "PointSet.h"
#include "PixelTracker.h"

using namespace std;

void Window::startCapture() {
    ParameterHandler* params = ParameterHandler::Instance();
    params->SetCaptureMode ( true );
    startCaptureButton->setEnabled ( false );
    controlWidget->setEnabled ( false );
    progressDialog = new QProgressDialog (
            QString ("Processing..."),
            QString ("Cancel"),
            0,
            100 
            );
    FileWriterServices* fws = FileWriterServices::Instance ();
    connect (
            fws, SIGNAL (       Progress ( int ) ),
            progressDialog, SLOT   (       setValue ( int ) )
            );
    connect (
            fws, SIGNAL (            Finished () ),
            this, SLOT   ( enableCaptureButton () )
            );
    progressDialog->show ();
}

void Window::enableCaptureButton () {
    startCaptureButton->setEnabled ( true );
    controlWidget->setEnabled ( true );
    if ( progressDialog ) {
        delete progressDialog;
        progressDialog = 0x0;
    }
    /* update list of frames*/
    updateFrameList();
    /*update combo box*/
    addImageItems();
}

void Window::updateFrameList () {

    /* Update the list of images in the system*/
    static const std::string IMAGE_LIST(" ls -B --ignore=*.txt --ignore=depth* --ignore=disparity* --ignore=*.ply " + Config::FramesPath() + " |  sed 's/f//g' | sed -r 's/^.{6}//' | sort -g >" + Config::FramesPath() + "list.txt");

    system(IMAGE_LIST.c_str());

}

void Window::sumShowingFrames(){
    showingFrames++;
    previousFrames -> setEnabled(true);
    updateAutoAnchorPreview ();
}

void Window::setReferenceFrame(int iFrame){

    int idxFrame = refFrameID % 15;
    if(refFrameID == iFrame){

        if(referenceFrame.at(idxFrame) -> frameStyle() != 18){
            referenceFrame.at(idxFrame) -> oldFrameStyle = referenceFrame.at(idxFrame) -> frameStyle();
            referenceFrame.at(idxFrame) -> setFrameStyle(QFrame::Panel | QFrame::Plain);
            referenceFrame.at(idxFrame) -> setStyleSheet("color:red");
            referenceFrame.at(idxFrame)  -> setLineWidth(3);
        }

        return;
    }

    if(refFrameID/15 == iFrame/15) {
        /* Change the old selected frame to unselected state*/
        if(referenceFrame.at(idxFrame) -> frameStyle() != 18){
            referenceFrame.at(idxFrame) -> oldFrameStyle = referenceFrame.at(idxFrame) -> frameStyle();
            referenceFrame.at(idxFrame) -> setFrameStyle(QFrame::Panel | QFrame::Plain);
            referenceFrame.at(idxFrame) -> setStyleSheet("color:red");
            referenceFrame.at(idxFrame)  -> setLineWidth(3);
        }
        else if(referenceFrame.at(idxFrame) -> oldFrameStyle == 17) {
            referenceFrame.at(idxFrame) -> setFrameStyle(QFrame::Box | QFrame::Plain);
            referenceFrame.at(idxFrame) -> setStyleSheet("color:blue");
            referenceFrame.at(idxFrame) -> setLineWidth(3);
        }
        else { 
            referenceFrame.at(idxFrame) -> setFrameStyle(referenceFrame.at(idxFrame) ->oldFrameStyle);
        }
    }  
    this -> refFrameID = iFrame;

}

void Window::subtractShowingFrames(){

    if(showingFrames > 0 ) showingFrames--;
    if(showingFrames == 0) previousFrames -> setDisabled(true);
    nextFrames->setEnabled(true);
    updateAutoAnchorPreview ();

}



void Window::setFarPlane ( const unsigned int& iFar ) {
    ParameterHandler* params = ParameterHandler::Instance ();
    params->SetFarPlane ( iFar );
}

void Window::setNearPlane ( const unsigned int& iNear ) {
    ParameterHandler* params = ParameterHandler::Instance ();
    params->SetNearPlane ( iNear );
}

void Window::findAutoAnchors(){

    ParameterHandler* params = ParameterHandler::Instance ();
    std::vector<int>     anchorFound;

    /* clear list of anchorFrames */
    if (!anchorFound.empty()){
        anchorFound.clear();
    }

    //TODO put a process dialog
    static const std::string ANCHOR_LIST_PATH(Config::FramesPath() + "anchorList.txt");
    static const std::string FRAME_LIST_PATH(Config::FramesPath() + "list.txt");

    QFile imageList(FRAME_LIST_PATH.c_str());
    QString id;

    /* Verify if the file with frames is readable*/
    if(!imageList.open(QIODevice::ReadOnly )) return;

    /*Creates a progress Dialog*/
    QProgressDialog* progressDialog = NULL;
    progressDialog = new QProgressDialog("Finding Anchor Frames...", "Cancel", 0,100);
    int progress = 0;

    if (progressDialog)
        progressDialog->show ();

    QTextStream listOfFrames(& imageList);

    Image refFrame(Config::FramesPath() + "f" + toString(refFrameID) + "/texture.pgm");
    cout << "Ref frame ID: "<<refFrameID << endl;

    while(!listOfFrames.atEnd())
    {
        progress +=  2;
        if (progressDialog)
            progressDialog->setValue (progress);

        id = listOfFrames.readLine();
        
        std::stringstream ss ( id.toStdString () );
        int i_id = 0;
        ss >> i_id;
        if (i_id == refFrameID) continue;

        Image frame(Config::FramesPath() + "f" + ss.str() + "/texture.pgm");	      

        float errorScore = ImageBase::CalculateErrorScore ( frame, refFrame );
        cout << errorScore << endl;
        if( errorScore < params->GetThreshold () ){
            cout << "Anchor frame found" << endl;
            /* Save frame as an anchor */
            bool ok;
            anchorFound.push_back(id.toInt(&ok,10));	  
        }

    }

    if (progressDialog)
        progressDialog->setValue (100);

    for(int j =0; j < anchorFound.size(); j++) {
        cout << anchorFound.at(j) << endl;
    }

    saveAnchors ( anchorFound ) ;
    updateAutoAnchorPreview ();

    if (progressDialog)
        delete progressDialog;


}

void  Window::setWindowSize(int iSize){
    ParameterHandler* params = ParameterHandler::Instance();
    params -> SetWindowSize(iSize);
}

void  Window::setNeighbourhoodSize(int  iSize){
    ParameterHandler* params = ParameterHandler::Instance();
    params -> SetNeighbourhoodSize(iSize);
}

void Window::setMesh(bool b){
    ParameterHandler* params = ParameterHandler::Instance();
    params -> SetMesh(b);

    if(b){
        viewer -> reset();
        centerWidget->setCurrentIndex ( viewerIdx );
    }
}

void Window::setDisplacement(bool b){
    if(b){
        updateImages();
        centerWidget->setCurrentIndex ( gridIdx );
    }
}

void Window::loadAnchorFrames(){

    /* clear list of anchorFrames */
    if (!isAnchorFrames.empty()){
        isAnchorFrames.clear();
    }

    static const std::string ANCHOR_LIST_PATH(Config::FramesPath() + "anchorList.txt");

    QFile anchorSavedList(ANCHOR_LIST_PATH.c_str());

    QString anchorID;

    /* Verify if the file with anchor frames is readable*/
    if(anchorSavedList.open(QIODevice::ReadOnly )){

        QTextStream anchor(& anchorSavedList);

        while(!anchor.atEnd())
        {
            anchorID = anchor.readLine();
            bool ok;
            int id = anchorID.toInt (&ok,10);

            isAnchorFrames.push_back(id);	  
        }
    }

    /* Close the file */
    anchorSavedList.close();

    return;
}

void Window::addAnchorListItems(){

    static const std::string IMG_LIST_PATH(Config::FramesPath() + "list.txt");
    static const std::string ANCHOR_LIST_PATH(Config::FramesPath() + "anchorList.txt");

    QFile imageList(IMG_LIST_PATH.c_str());
    QFile anchorSavedList(ANCHOR_LIST_PATH.c_str());

    QString id;
    QString anchorID;

    /* Verify if the file with frames is readable*/
    if(!imageList.open(QIODevice::ReadOnly )) return;

    /* Verify if the file with anchor frames is readable*/
    if(!anchorSavedList.open(QIODevice::ReadOnly )){
        QTextStream in(& imageList);

        while(!in.atEnd())
        {
            id = in.readLine();

            /* Code used for sorting */
            QString sortableID = "0";
            if(id.size() == 1){ 
                sortableID.append(id);
            }
            else{
                sortableID = id;
            }
            /* Show the ID in the list box*/
            candidateAnchorList->addItem(sortableID);
        }
    }

    else {

        QTextStream in(& imageList);
        QTextStream anchor(& anchorSavedList);

        if(!anchor.atEnd())
            anchorID = anchor.readLine();

        while(!in.atEnd())
        {
            id = in.readLine();

            /* Code used for sorting */
            QString sortableID = "0";
            if(id.size() == 1){ 
                sortableID.append(id);
            }
            else{
                sortableID = id;
            }

            /* Show the ID in the combo box*/
            if(!id.compare(anchorID)){ 
                anchorList->addItem(sortableID);
                anchorID = anchor.readLine();
            }
            else{
                candidateAnchorList->addItem(sortableID);
            }
        }

        anchorSavedList.close();

    }

    /* Close the file */
    imageList.close();

    return;

}

void Window::saveAnchors(){
    //TODO: Create a file with the items in anchorList
    static const std::string ANCHOR_LIST_PATH(Config::FramesPath() + "anchorList.txt");

    QFile anchorFile(ANCHOR_LIST_PATH.c_str());
 
    /* Verify if the file readable*/
    if(!anchorFile.open(QIODevice::WriteOnly))
        return;

    for(int i= 0; i < anchorList -> count(); i++){
        bool ok;
        QString id(QString::number(anchorList -> item(i) -> text().toInt(&ok,10)));
        id.append("\n");
        anchorFile.write(id.toUtf8());
    }

    anchorFile.close();
}
void Window::saveAnchors(
    const std::vector<int>& iAnchorList
){
    //TODO: Create a file with the items in anchorList
    static const std::string ANCHOR_LIST_PATH(Config::FramesPath() + "anchorList.txt");

    QFile anchorFile(ANCHOR_LIST_PATH.c_str());

    /* Verify if the file readable*/
    if(!anchorFile.open(QIODevice::WriteOnly))
        return;

    for(int i= 0; i < iAnchorList.size(); i++){
        QString id(QString::number(iAnchorList.at(i)));
        QString line = id.append("\n");
        anchorFile.write(line.toUtf8 ());
    }

    anchorFile.close();

}

void Window::updateManuAnchorPreview(){

    std::string RES_IMG_PATH(Config::FramesPath());
    /* Get list of selected items, we will preview only the first one in the list*/
    QList<QListWidgetItem *>  candidates = candidateAnchorList -> selectedItems();
    QList<QListWidgetItem *>  anchors = anchorList -> selectedItems();

    QString id1,id2;
    if(candidates.count() > 0){
        id1 = (candidates[0] -> text());
    }
    else{
        id1 = "-1";
    }
    if(anchors.count() > 0){
        id2 = (anchors[0] -> text());
    }
    else{
        id2 = "-1";
    }

    /* Removes 0 from the left to align with write file names pattern */
    bool ok;
    int id = id1.toInt (&ok,10);
    std::string frameID1 = toString(id);
    id = id2.toInt   (&ok,10);
    std::string frameID2= toString(id);


    QPixmap anchorCandidateImg(QString::fromUtf8(((Config::FramesPath () + "f" + frameID1 + "/texture.pgm").c_str())));
    if(!anchorCandidateImg.isNull())
        anchorCandidate -> setPixmap(anchorCandidateImg.scaled(180, 150, Qt::IgnoreAspectRatio, Qt::FastTransformation));

    QPixmap anchorImg(QString::fromUtf8(((Config::FramesPath () + "f" + frameID2 + "/texture.pgm").c_str())));
    if(!anchorImg.isNull()){
        anchor -> setPixmap(anchorImg.scaled(180, 150, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }

}

void Window::updateAutoAnchorPreview(){

    std::string RES_IMG_PATH(Config::FramesPath());

    loadAnchorFrames();

    for(int i = 0; i < 15;  i++){
        referenceFrame.at(i) -> setFrameStyle(QFrame::NoFrame);
        referenceFrame.at(i) -> setVisible(true);
    }

    for(int i = 15 * showingFrames, j = 0; i < 15 + 15 * showingFrames; i++,j++){

        QPixmap anchorCandidateImg(QString::fromUtf8(((Config::FramesPath () + "f"+ toString ( i ) + "/texture.pgm").c_str())));

        if(!anchorCandidateImg.isNull()){
            referenceFrame.at(j) -> setPixmap(anchorCandidateImg.scaled(100, 80, Qt::IgnoreAspectRatio, Qt::FastTransformation));
            referenceFrame.at(j) -> frameID = i;

            if(std::find(isAnchorFrames.begin(), isAnchorFrames.end(), i)!=isAnchorFrames.end())
            {
                referenceFrame.at(j) -> setFrameStyle(QFrame::Box | QFrame::Plain);
                referenceFrame.at(j) -> setStyleSheet("color:blue");
                referenceFrame.at(j) -> setLineWidth(3);
            }
            /* Default selected Frame */
            if(i == refFrameID){
                referenceFrame.at(j) -> oldFrameStyle = referenceFrame.at(j) -> frameStyle();
                referenceFrame.at(j) -> setFrameStyle(QFrame::Panel | QFrame::Plain);
                referenceFrame.at(j) -> setStyleSheet("color:red");
                referenceFrame.at(j)  -> setLineWidth(3);

            }

        }
        else{
            referenceFrame.at(j) -> clear();
            referenceFrame.at(j) ->setVisible(false);
            /*BUG: if nb of frames % 15 == 0, next frames has to be pressed 2 times */
            nextFrames -> setDisabled(true);
        }
    }
}

void Window::initManuAnchorSelection(){

    QLabel *candidateAnchorCaption =  new QLabel("List of no-anchors frames:",anchorManuSelection);
   candidateAnchorCaption -> setGeometry(QRect(52, 205, 180, 21));

    if (candidateAnchorList) delete candidateAnchorList;
    candidateAnchorList = new QListWidget(anchorManuSelection);
    candidateAnchorList -> setGeometry(QRect(50, 230, 180, 150));

    QLabel *anchorListCaption =  new QLabel("List of anchors frames:",anchorManuSelection);
    anchorListCaption -> setGeometry(QRect(373, 205, 180, 21));

    if (anchorList) delete anchorList;
    anchorList = new QListWidget(anchorManuSelection);
    anchorList -> setGeometry(QRect(371, 230, 180, 150));

    addAnchorListItems();

    if (anchorCandidate) delete anchorCandidate;
    anchorCandidate = new QLabel(anchorManuSelection);
    anchorCandidate -> setGeometry(QRect(50, 50, 180, 150));
    anchorCandidate -> setMaximumSize(QSize(180, 150));

    if (anchor) delete anchor;
    anchor = new QLabel(anchorManuSelection);
    anchor -> setGeometry(QRect(371, 50, 180, 150));
    anchor -> setMaximumSize(QSize(180, 150));

    updateManuAnchorPreview ();

    connect(
            candidateAnchorList, SIGNAL(    itemSelectionChanged () ),
            this, SLOT  ( updateManuAnchorPreview () )
           );

    connect(
            anchorList, SIGNAL(    itemSelectionChanged () ),
            this, SLOT  ( updateManuAnchorPreview () )
           );

}

void Window::setThreshold( double iThreshold){
    ParameterHandler* params = ParameterHandler::Instance();
    params -> SetThreshold(iThreshold);
}

void Window::initAutoAnchorSelection(){

    loadAnchorFrames();

    /* Frames to be showed while selecting */
    for(int i = 0; i < 15; i++){
        referenceFrame << new AnchorLabel(anchorAutoSelection);

        connect(
            referenceFrame.at(i), SIGNAL(     mousePressed (int) ), 
                            this, SLOT  ( setReferenceFrame(int) ) 
        );
    }

    /* Places all qlabel in the UI three lines and five columns*/
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 5; j++){
            referenceFrame.at(i*5+j) -> setGeometry(QRect(50 + 120 * j, 50 + 90 * i, 100, 80));
            referenceFrame.at(i*5+j) -> setMaximumSize(QSize(100, 80));
        }
    }
    
    /* Create buttons to interact with frames */
    previousFrames =  new QPushButton ("Previous Frames", anchorAutoSelection);
    previousFrames -> setGeometry( QRect(480, 340, 110, 25) );
    previousFrames -> setDisabled(true);
    showingFrames  =  0;
    nextFrames     =  new QPushButton ("Next Frames", anchorAutoSelection);
    nextFrames     -> setGeometry( QRect(480, 375, 110, 25) );
    findAnchors    =  new QPushButton ("Find Anchors", anchorAutoSelection);
    findAnchors    -> setGeometry( QRect(250, 350, 140, 31) );
    QLabel         *  thresholdLabel = new QLabel(tr("Threshold:"),anchorAutoSelection);
    thresholdLabel->setGeometry(QRect(250, 390, 75, 33));
    thresholdSP =  new QDoubleSpinBox(anchorAutoSelection);
    thresholdSP -> setGeometry(QRect(330, 390, 62, 25));
    thresholdSP -> setMaximum(1000000);
    thresholdSP -> setSingleStep(0.01);
    thresholdSP -> setDecimals ( 3 );
    ParameterHandler* params = ParameterHandler::Instance();
    thresholdSP -> setValue(params -> GetThreshold());


    /* Creating caption/explanation */
    QPalette palette;
    QBrush blue(QColor(0, 0, 255, 255));
    blue.setStyle(Qt::SolidPattern);
    QBrush red(QColor(255, 0, 0, 255));
    red.setStyle(Qt::SolidPattern);
    QBrush green(QColor(0, 255, 0, 255));
    green.setStyle(Qt::SolidPattern);

    /* blue caption*/
    palette.setBrush(QPalette::Active, QPalette::Window, blue);
    QLabel *anchorCaptionText =  new QLabel("Saved Anchors ",anchorAutoSelection);
    anchorCaptionText -> setGeometry(QRect(80, 340, 100, 21));
    QLabel *anchorCaptionBox =  new QLabel(anchorAutoSelection);
    anchorCaptionBox -> setGeometry(QRect(50, 340, 21, 21));
    anchorCaptionBox -> setPalette(palette);
    anchorCaptionBox -> setAutoFillBackground(true);

    /* green caption*/
    /*
    palette.setBrush(QPalette::Active, QPalette::Window, green);
    QLabel *autoAnchorCaptionText =  new QLabel("Automatically selected",anchorAutoSelection);
    autoAnchorCaptionText -> setGeometry(QRect(80, 370, 150, 21));
    QLabel *autoAnchorCaptionBox =  new QLabel(anchorAutoSelection);
    autoAnchorCaptionBox -> setGeometry(QRect(50, 370, 21, 21));
    autoAnchorCaptionBox -> setPalette(palette);
    autoAnchorCaptionBox -> setAutoFillBackground(true);
    */

    /* red caption*/
    palette.setBrush(QPalette::Active, QPalette::Window, red);
    QLabel *referenceFrameCaptionText =  new QLabel("Reference frame",anchorAutoSelection);
    referenceFrameCaptionText -> setGeometry(QRect(80, 370, 200, 21));
    QLabel *referenceFrameCaptionBox =  new QLabel(anchorAutoSelection);
    referenceFrameCaptionBox -> setGeometry(QRect(50, 370, 21, 21));
    referenceFrameCaptionBox -> setPalette(palette);
    referenceFrameCaptionBox -> setAutoFillBackground(true);

    /* Creating connections */
    connect(
            previousFrames, SIGNAL(               clicked () ),
            this, SLOT  ( subtractShowingFrames () )
           );
    connect(
            nextFrames, SIGNAL(          clicked () ),
            this, SLOT  ( sumShowingFrames () )
           );

    connect(
            findAnchors, SIGNAL(          clicked () ),
            this, SLOT  (  findAutoAnchors () )
           );

    connect(
            thresholdSP, SIGNAL(     valueChanged (double) ),
            this, SLOT  (     setThreshold (double) )
           );

    updateAutoAnchorPreview ();

}

void Window::setManuAnchor(bool b){
    if(b){
        initManuAnchorSelection();
        centerWidget -> setCurrentIndex ( anchorManuIdx);
    }
    else{ 
        //save anchors listed in file 
        saveAnchors();
    }

}

void Window::setAutoAnchor(bool b){
    if(b){
        initAutoAnchorSelection();
        centerWidget -> setCurrentIndex ( anchorAutoIdx);
    }
    else{ 
        //save anchors listed in file 
        //saveAnchors();
    }

}

/*!
 *  \brief  Set the new scene selected from the box
 *  \param  scene Number of the scene
 */
void Window::setFrame1(int iFrame) {
    ParameterHandler* params = ParameterHandler::Instance();

    if(iFrame >= 0 && !isinf(iFrame)){
        params -> SetFrame1(iFrame);

        std::string path = Config::FramesPath() + "f" + toString (iFrame) + "/";

        Frame* frame = new Frame ();
        frame->LoadFromFile ( path );
        params->SetCurrentFrame ( frame );
    }
    
    if(params -> GetMesh()){
        viewer -> reset();
        viewer -> updateGL();
    }
    else{
        updateImages();
    }
}

/* Move Item from candidate list to anchor List*/
void Window::addNewAnchorItem(){

    /*Get selected row*/
    int row = candidateAnchorList -> currentRow();

    /*Remove row from list and add to another list */
    QListWidgetItem * selectedFrame = candidateAnchorList ->  takeItem(row);
    anchorList -> addItem(selectedFrame -> text());

    /* Sort list*/
    anchorList -> sortItems();
}

/* Move item from anchor list to candidate List*/
void Window::removeAnchorItem(){

    /*Get selected row*/
    int row = anchorList -> currentRow();

    /*Remove row from list and add to another list */
    QListWidgetItem * selectedFrame = anchorList ->  takeItem(row);

    candidateAnchorList -> addItem(selectedFrame -> text());

    /* Sort list*/
    candidateAnchorList -> sortItems();

}

void Window::calcDisp() {
    ParameterHandler* params = ParameterHandler::Instance();

    const std::string f1path =  Config::FramesPath() + "f" + Int2Str(params->GetFrame1()) + "/";
    const std::string f2path =  Config::FramesPath() + "f" + Int2Str(params->GetFrame2()) + "/";

    PointSet* refMesh = new PointSet ( f1path + "mesh.ply" );
    PointSet* tarMesh = new PointSet ( f2path + "mesh.ply" );
    Image* refImg = new Image ( f1path + "texture.pgm"); 
    Image* tarImg = new Image ( f2path + "texture.pgm");
    Image* refDep = new Image ( f1path + "depthMap.pgm");
    Image* tarDep = new Image ( f2path + "depthMap.pgm");

    PixelTracker pt (0);
    pt.SetReference (
        params->GetFrame1 (),
        refImg,
        refDep
    );
    pt.SetTarget (
        params->GetFrame2 (),
        tarImg,
        tarDep
    );
    pt.Track ();
    pt.Calculate3DDisplacements (
       refMesh, tarMesh 
    );
    pt.Export ( f1path );
    
    std::cout << "Frame ID: " << Int2Str(params->GetFrame1())<< std::endl;
    params->GetCurrentFrame()->LoadFromFile ( f1path );

    delete refImg;
    refImg = (Image*)0x0;
    delete tarImg;
    tarImg = (Image*)0x0;
    delete refDep;
    refDep = (Image*)0x0;
    delete tarDep;
    tarDep = (Image*)0x0;


    //const unsigned int& wSize = params->GetWindowSize ();
    //const unsigned int& nSize = params->GetNeighbourhoodSize ();

    //std::string RES_IMG_PATH(Config::FramesPath());
    //std::string frameID1 = toString(params -> GetFrame1());
    //std::string frameID2 = toString(params -> GetFrame2());

    //Image frame1(RES_IMG_PATH + "image_"+ frameID1 + ".pgm");
    //Image frame2(RES_IMG_PATH + "image_"+ frameID2 + ".pgm");
    //Image dispX( frame1.GetWidth(), frame1.GetHeight(), wSize );
    //Image dispY( frame1.GetWidth(), frame1.GetHeight(), wSize );

    //try {
    //    Image::TrackPixels (
    //            frame1,
    //            frame2,
    //            wSize,
    //            wSize,
    //            nSize,
    //            nSize,
    //            dispX,
    //            dispY
    //            );
    //    dispX.CreateAsciiPgm(Config::OutputPath() + "TrackingF"+ frameID1 + "F"+ frameID2+"x.pgm");
    //    dispY.CreateAsciiPgm(Config::OutputPath() + "TrackingF"+ frameID1 + "F"+ frameID2+"y.pgm");
    //} catch (BadIndex bi) {
    //    std::cout << bi.what();
    //}
    //updateImages ();
}

void Window::updateImages() {
    ParameterHandler* params = ParameterHandler::Instance();
    std::string RES_IMG_PATH(Config::FramesPath());
    std::string frameID1 = toString(params -> GetFrame1());
    std::string frameID2 = toString(params -> GetFrame2());

    /* Reads image 1 */
    if (img1) delete img1;
    img1 = new QLabel;
    img1 -> setMaximumSize(QSize(320, 240));
    QPixmap pic1(QString::fromUtf8(((RES_IMG_PATH + "f"+ frameID1 + "/texture.pgm").c_str())));
    if(!pic1.isNull())
        img1 -> setPixmap(pic1.scaled(320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));

    /* Reads image 2 */
    if (img2) delete img2;
    img2 = new QLabel;
    img2 -> setMaximumSize(QSize(320, 240));
    QPixmap pic2(QString::fromUtf8(((RES_IMG_PATH + "f"+ frameID2 + "/texture.pgm").c_str())));
    if(!pic2.isNull())
        img2 -> setPixmap(pic2.scaled(320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));

    /* Reads X displacement */
    if (dispX) delete dispX;
    dispX = new QLabel;
    dispX -> setMaximumSize(QSize(320, 240));
    QPixmap pic3(QPixmap(QString::fromUtf8(((Config::OutputPath() + "TrackingF"+ frameID1 + "F"+ frameID2 + "x.pgm").c_str()))));
    if(!pic3.isNull())
        dispX -> setPixmap(pic3.scaled( 320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));

    /* Reads Y displacement */
    if (dispY) delete dispY;
    dispY = new QLabel;
    dispY -> setMaximumSize(QSize(320, 240));
    QPixmap pic4(QPixmap(QString::fromUtf8(((Config::OutputPath() + "TrackingF"+ frameID1 + "F"+ frameID2 + "y.pgm").c_str()))));
    if(!pic4.isNull())
        dispY -> setPixmap(pic4.scaled( 320, 240, Qt::IgnoreAspectRatio, Qt::FastTransformation));

    /* Set image in a 2x2 grid*/
    if (gridLayout) delete gridLayout;
    gridLayout = new QGridLayout(gridLayoutWidget);
    gridLayout -> setContentsMargins(0, 0, 0, 0);
    gridLayout -> addWidget( img1, 0, 0, 1, 1);
    gridLayout -> addWidget( img2, 0, 1, 1, 1);
    gridLayout -> addWidget(dispX, 1, 0, 1, 1);
    gridLayout -> addWidget(dispY, 1, 1, 1, 1);
}

void Window::setFrame2(int iFrame) {
    ParameterHandler* params = ParameterHandler::Instance();
    if(iFrame >= 0 && !isinf(iFrame))
        params->SetFrame2(iFrame);
    updateImages();
}

/*!
 *  \brief Save image of the GLViewer
 */
void Window::saveGLImage () {
    viewer->saveSnapshot (false, false);
}

/*!
 *  \brief Get all the frames id from the directory and add to comboBox 
 */
void Window::addImageItems()
{
    static const std::string IMG_LIST_PATH(Config::FramesPath() + "list.txt");

    QFile imageList(IMG_LIST_PATH.c_str());
    QString fileName;
    
    /* Clear combo box items*/
    frame1ComboBox -> clear();
    frame2ComboBox -> clear();

    /* Verify if the file readable*/
    if(!imageList.open(QIODevice::ReadOnly ))
        return;

    /* Reads the file */
    QTextStream in(& imageList);

    while(!in.atEnd())
    {
        fileName = in.readLine();
        /* Show the ID in the combo box*/
        frame1ComboBox -> addItem(fileName,QVariant::Char); 
        frame2ComboBox -> addItem(fileName,QVariant::Char); 
    }

    /* Close the file */
    imageList.close();
    return;
}

/*!
 *  \brief  Creates the UI (upper menu, left and right dock and GLViewer)
 */
Window::Window ()
    :   QMainWindow (NULL),
    centerWidget(NULL),
    controlWidget(NULL),
    createMeshPB(NULL),
    calcDispPB(NULL),
    snapshotButton(NULL),
    startCaptureButton(NULL),
    frame1ComboBox(NULL),
    frame2ComboBox(NULL),
    viewer(NULL),
    viewerIdx(0),
    cameraTimer(NULL),
    progressDialog(NULL),
    img1(NULL),
    img2(NULL),
    dispX(NULL),
    dispY(NULL),
    gridLayoutWidget(NULL),
    gridIdx(0),
    gridLayout(NULL),
    anchorList(NULL),
    candidateAnchorList(NULL),
    addAnchor(NULL),
    removeAnchor(NULL),
    anchor(NULL),
    anchorCandidate(NULL),
    anchorManuSelection(NULL),
    anchorManuIdx(0),
    //        referenceFrame(NULL),
    anchorAutoSelection(NULL),
    anchorAutoIdx(0),
    showingFrames(0),
    thresholdSP(NULL)
{
    /* Update the list of frames captured*/
    updateFrameList();

    try {
        viewer = new GLViewer;
    } catch (GLViewer::Exception e) {
        cerr << e.getMessage () << endl;
        exit (1);
    }
    //    connect(this, SIGNAL(frameChanged()), viewer,SLOT(update()));
    gridLayoutWidget = new QWidget (this);
    anchorManuSelection  = new QWidget (this);
    anchorAutoSelection  = new QWidget (this);

    centerWidget = new QStackedWidget();
    setCentralWidget (centerWidget);

    viewerIdx     = centerWidget -> addWidget (                viewer );
    gridIdx       = centerWidget -> addWidget (      gridLayoutWidget );
    anchorManuIdx = centerWidget -> addWidget (   anchorManuSelection );
    anchorAutoIdx = centerWidget -> addWidget (   anchorAutoSelection );

    /* Adding settings to upper menu */
    /* Adding quit and about buttons to upper menu */
    QMenu *fileMenu = menuBar()->addMenu(tr("&Help"));

    QAction *glViewerHelp = new QAction(tr("&GLViewer Help"), this);
    fileMenu -> addAction(glViewerHelp);

    connect(
            glViewerHelp, SIGNAL( triggered() ),
            viewer, SLOT  (      help() )
           );

    QAction *openAct = new QAction(tr("&About..."), this);
    fileMenu -> addAction(openAct);

    connect(
            openAct, SIGNAL( triggered() ),
            this   , SLOT  (     about() )
           );

    fileMenu -> addSeparator();

    QAction *exitAct = new QAction(tr("E&xit"), this);
    fileMenu -> addAction(exitAct);

    connect(
            exitAct, SIGNAL(      triggered() ),
            this, SLOT  ( exitPreprocess() )
           );

    connect(
            this, SIGNAL(         exiting() ),
            qApp, SLOT  ( closeAllWindows() )
           );

    createDock();
    statusBar()->showMessage("");

}

void Window::exitPreprocess () {
    FileWriterServices::DeleteInstance ();
    emit ( exiting () );
}

void Window::createDock () {

    /* Defining size policy of the windows */
    QSizePolicy sizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    /* Left Doc: exection buttons */
    controlDockWidget = new QDockWidget (this);
    initControlWidget ();
    controlDockWidget->setWidget (controlWidget);
    sizePolicy.setHeightForWidth(controlDockWidget->sizePolicy().hasHeightForWidth());
    controlDockWidget->setSizePolicy(sizePolicy);
    addDockWidget (Qt::LeftDockWidgetArea, controlDockWidget);
    controlDockWidget->setFeatures (QDockWidget::AllDockWidgetFeatures);

}
Window::~Window () {

}

/*!
 *  \brief  GLviewer help
 */
void Window::GLViewerHelp () {
}


/*!
 *  \brief  Show a content about this program 
 */
void Window::about () {
    QMessageBox::about (this, 
            "About This Program", 
            "<b>PIM380</b>"
            "<br> by <i>Vinicius Dias Gardelli</i> "
            "<br> and <i>Tiago Chedraoui Silva</i>.");
}


void Window::initControlWidget () {

    /* Get initial defined parameters*/
    ParameterHandler* params = ParameterHandler::Instance();

    /* Defining size policy of the windows */
    QSizePolicy sizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    controlWidget = new QGroupBox ();
    QVBoxLayout * layout = new QVBoxLayout (controlWidget);

    QGroupBox *  previewGroupBox = new QGroupBox   ("Preview", controlWidget);
    QVBoxLayout *   previewLayout = new QVBoxLayout (previewGroupBox);

    /* Creating tables for frame selection */
    frame1ComboBox = new QComboBox (previewGroupBox);
    frame2ComboBox = new QComboBox (previewGroupBox);
    this -> setFrame1(0); // Used for point set allocation
    addImageItems();

    QLabel          *   frame1Label = new QLabel(tr("Frame:"));
    frame1Label -> setBuddy(frame1ComboBox);

    QLabel          *   frame2Label = new QLabel(tr("Frame 2:"));
    frame2Label -> setBuddy(frame2ComboBox);

    QWidget *generalLayoutWidget   = new QWidget(previewGroupBox);
    QFormLayout *generalFormLayout = new QFormLayout(generalLayoutWidget);
    generalFormLayout -> setContentsMargins(0, 0, 0, 0);
    generalFormLayout -> setWidget(0, QFormLayout::LabelRole, frame1Label);
    generalFormLayout -> setWidget(0, QFormLayout::FieldRole, frame1ComboBox);
    generalFormLayout -> setWidget(1, QFormLayout::LabelRole, frame2Label);
    generalFormLayout -> setWidget(1, QFormLayout::FieldRole, frame2ComboBox);

    /* Creation of left buttons*/
    createMeshPB       = new QPushButton ("Create Mesh", previewGroupBox);
    calcDispPB         = new QPushButton ("Calc Displacement", previewGroupBox);
    snapshotButton     = new QPushButton ("Save preview", previewGroupBox);
    startCaptureButton = new QPushButton ("Start Capture", previewGroupBox);

    /*Creating radio buttons*/
    QButtonGroup * modeButtonGroup = new QButtonGroup (previewGroupBox);
    modeButtonGroup -> setExclusive (true);
    displacementRB  =  new QRadioButton("Displacement", previewGroupBox);
    meshRB          =  new QRadioButton("Mesh"        , previewGroupBox);
    anchorManuRB    =  new QRadioButton("Edit Anchor" , previewGroupBox);
    anchorAutoRB    =  new QRadioButton("Auto Anchor" , previewGroupBox);
    modeButtonGroup -> addButton (displacementRB);
    modeButtonGroup -> addButton (meshRB);
    modeButtonGroup -> addButton (anchorManuRB);
    modeButtonGroup -> addButton (anchorAutoRB);

    /* Anchor buttons*/
    addAnchor    = new QPushButton ( ">", anchorManuSelection );
    removeAnchor = new QPushButton ( "<", anchorManuSelection );
    addAnchor    -> setGeometry( QRect(280, 260, 41, 31) );
    removeAnchor -> setGeometry( QRect(280, 300, 41, 31) );


    QGroupBox *  parametersGroupBox = new QGroupBox   ("Parameters", controlWidget);
    QVBoxLayout *   parametersLayout = new QVBoxLayout (parametersGroupBox);

    windowSizeSP = new QSpinBox(parametersGroupBox);
    neighbourhoodSizeSP = new QSpinBox(parametersGroupBox);
    neighbourhoodSizeSP -> setValue(params -> GetNeighbourhoodSize());
    windowSizeSP -> setValue(params -> GetWindowSize());
    windowSizeSP -> setRange(1,30);
    neighbourhoodSizeSP -> setRange(1,30);

    QLabel          *   windowSizeLabel = new QLabel(tr("Window Size:"));
    frame1Label -> setBuddy(neighbourhoodSizeSP);

    QLabel          *   neighbourhoodSizeSPLabel = new QLabel(tr("Neighbourhood:"));
    frame2Label -> setBuddy(windowSizeSP);

    QWidget *paramsLayoutWidget   = new QWidget(parametersGroupBox);
    QFormLayout *paramsFormLayout = new QFormLayout(paramsLayoutWidget);
    paramsFormLayout -> setContentsMargins(0, 0, 0, 0);
    paramsFormLayout -> setWidget(0, QFormLayout::LabelRole, windowSizeLabel);
    paramsFormLayout -> setWidget(0, QFormLayout::FieldRole, windowSizeSP);
    paramsFormLayout -> setWidget(1, QFormLayout::LabelRole, neighbourhoodSizeSPLabel);
    paramsFormLayout -> setWidget(1, QFormLayout::FieldRole, neighbourhoodSizeSP);


    /********** Connections ***********/

    /*** Situation: Mesh showing *****/
    /* Description: TODO */
    connect (
            createMeshPB, SIGNAL (       clicked () ),
            this, SLOT   (    createMesh () )
            );

    /* Description: Print screen */
    connect (
            snapshotButton, SIGNAL (      clicked () ),
            this, SLOT   (  saveGLImage () )
            );

    /* Description: Capture Frames */
    connect (
            startCaptureButton, SIGNAL (      clicked () ),
            this, SLOT   ( startCapture () )
            );

    /* Description: Disabling image 2 selection */
    //connect(
    //        meshRB, SIGNAL (     toggled (bool) ), 
    //        frame2ComboBox, SLOT   ( setDisabled (bool) )
    //       );

    /* Description: Disabling calculate displacement push button */
    //connect(
    //        meshRB, SIGNAL (       toggled (bool) ), 
    //        calcDispPB, SLOT   (   setDisabled (bool) )
    //       );

    /* Description: Change of situation 
       Mesh selected -> update screen */
    connect(
            meshRB, SIGNAL( toggled (bool) ), 
            this, SLOT  ( setMesh (bool) )
           );

    /*** Situation: Displacement showing *****/
    /* Description: Calculate displacement */
    connect (
            calcDispPB, SIGNAL (      clicked  () ), 
            this, SLOT   (      calcDisp () )
            );

    /* Description: Disabling Mesh snapshot */
    connect(
            displacementRB, SIGNAL (      toggled (bool) ),
            snapshotButton, SLOT   (  setDisabled (bool) )
           );


    /* Description: Disabling capture button */
    connect(
            displacementRB, SIGNAL (     toggled (bool) ),
            startCaptureButton, SLOT   ( setDisabled (bool) )
           );

    /* Description: Disabling create mesh */
    connect(
            displacementRB, SIGNAL (     toggled (bool) ),
            createMeshPB, SLOT   ( setDisabled (bool) )
           );

    /* Description: Changing Frame 1 */
    connect (
            frame1ComboBox, SIGNAL ( currentIndexChanged (int) ), 
            this, SLOT   (           setFrame1 (int) )
            );

    /* Description: Changing Frame 2 */
    connect (
            frame2ComboBox, SIGNAL ( currentIndexChanged (int) ), 
            this, SLOT   (           setFrame2 (int) )
            );

    /* Description: Change of situation 
       Displacement selected -> update screen */
    connect(
            displacementRB, SIGNAL (         toggled (bool) ),
            this, SLOT   ( setDisplacement (bool) )
           );


    /*** Situation: Anchor manual selection *****/

    /* Description: add candidate frame as anchor */
    connect (
            addAnchor, SIGNAL (         clicked  () ), 
            this, SLOT   ( addNewAnchorItem () )
            );

    /* Description: remove frame from anchor */
    connect (
            removeAnchor, SIGNAL (         clicked  () ), 
            this, SLOT   ( removeAnchorItem () )
            );

    /* Description: Change of situation 
       Displacement selected -> update screen */
    connect(
            anchorManuRB, SIGNAL (         toggled (bool) ),
            this, SLOT   (       setManuAnchor (bool) )
           );

    /* Description: Disabling calculate displacement push button */
    connect(
            anchorManuRB, SIGNAL (       toggled (bool) ), 
            calcDispPB, SLOT   (   setDisabled (bool) )
           );


    /* Description: Disabling Mesh snapshot */
    connect(
            anchorManuRB, SIGNAL (      toggled (bool) ),
            snapshotButton, SLOT   (  setDisabled (bool) )
           );


    /* Description: Disabling capture button */
    connect(
            anchorManuRB, SIGNAL (     toggled (bool) ),
            startCaptureButton, SLOT   ( setDisabled (bool) )
           );

    /* Description: Disabling create mesh */
    connect(
            anchorManuRB, SIGNAL (     toggled (bool) ),
            createMeshPB, SLOT   ( setDisabled (bool) )
           );

    /* Description: Disabling frame1 selection */
    connect(
            anchorManuRB, SIGNAL (     toggled (bool) ),
            frame1ComboBox, SLOT   ( setDisabled (bool) )
           );

    /* Description: Disabling frame 2 semection */
    connect(
            anchorManuRB, SIGNAL (     toggled (bool) ),
            frame2ComboBox, SLOT   ( setDisabled (bool) )
           );


    /*** Situation: Anchor automatic selection *****/

    /* Description: Change of situation 
       Displacement selected -> update screen */
    connect(
            anchorAutoRB, SIGNAL (         toggled (bool) ),
            this, SLOT   (       setAutoAnchor (bool) )
           );

    /* Description: Disabling calculate displacement push button */
    connect(
            anchorAutoRB, SIGNAL (       toggled (bool) ), 
            calcDispPB, SLOT   (   setDisabled (bool) )
           );


    /* Description: Disabling Mesh snapshot */
    connect(
            anchorAutoRB, SIGNAL (      toggled (bool) ),
            snapshotButton, SLOT   (  setDisabled (bool) )
           );


    /* Description: Disabling capture button */
    connect(
            anchorAutoRB, SIGNAL (     toggled (bool) ),
            startCaptureButton, SLOT   ( setDisabled (bool) )
           );

    /* Description: Disabling create mesh */
    connect(
            anchorAutoRB, SIGNAL (     toggled (bool) ),
            createMeshPB, SLOT   ( setDisabled (bool) )
           );
    /* Description: Disabling frame1 selection */
    connect(
            anchorAutoRB, SIGNAL (     toggled (bool) ),
            frame1ComboBox, SLOT   ( setDisabled (bool) )
           );

    /* Description: Disabling frame 2 semection */
    connect(
            anchorAutoRB, SIGNAL (     toggled (bool) ),
            frame2ComboBox, SLOT   ( setDisabled (bool) )
           );

    /*** Situation: Parameters modification *****/

    /* Description: Change window size */
    connect (
            windowSizeSP, SIGNAL (valueChanged  (int) ), 
            this, SLOT   (setWindowSize (int) )
            );

    /* Description: Change window size */
    connect (
            neighbourhoodSizeSP, SIGNAL (       valueChanged  (int) ), 
            this, SLOT   (setNeighbourhoodSize (int) )
            );

    /*** Initial situation: Default options ***/
    meshRB         -> setChecked(true);
    //calcDispPB     -> setDisabled(true);
    //frame2ComboBox -> setDisabled(true);


    /* Verify if a camera is connect */
    if(params -> GetCamera()){

        cameraTimer = new QTimer();
        Camera* cam = &(Camera::Instance());

        connect (
                cameraTimer, SIGNAL (          timeout () ),
                cam, SLOT   ( WaitUpdateCamera () )
                );
        connect (
                cameraTimer, SIGNAL (          timeout () ),
                viewer     , SLOT   (           update () )
                );
        cameraTimer->start(16);
    }
    else {
        /* No device found: disable glviewer */
        meshRB -> setChecked(false);
        //meshRB -> setDisabled(true);
        displacementRB -> setChecked(true);
        anchorManuRB -> setChecked(false);
        anchorAutoRB -> setChecked(false);
        setDisplacement(true);
    }

    FileWriterServices* fws = FileWriterServices::Instance ();

    /* Add widgets to layout*/
    previewLayout->addWidget (generalLayoutWidget);
    previewLayout->addWidget (displacementRB);
    previewLayout->addWidget (meshRB);
    previewLayout->addWidget (anchorManuRB);
    previewLayout->addWidget (anchorAutoRB);
    previewLayout->addWidget (createMeshPB);
    previewLayout->addWidget (calcDispPB);
    previewLayout->addWidget (snapshotButton);
    previewLayout->addWidget (startCaptureButton);
    parametersLayout->addWidget (paramsLayoutWidget);

    /* Add widgets to left dock layout*/
    layout -> addWidget (previewGroupBox);
    layout -> addWidget (parametersGroupBox);
    layout -> addStretch (0);
}

/*!
 *  \brief  Create the vertex of a mesh from the image and deeper image 
 */
void Window::createMesh () {
    Camera* cam = &(Camera::Instance());

    QFileDialog *filedialog=new QFileDialog();
    filedialog->setAcceptMode(QFileDialog::AcceptSave);//To save files,default is open mode.
    QString fileAbsPath = QFileDialog::getSaveFileName(this);
    qDebug() << fileAbsPath;
    cam -> captureSingleMesh(fileAbsPath.toStdString());
}

void AnchorLabel::enterEvent (
    QEvent* e
) {
    emit mouseEntered ();
}
void AnchorLabel::leaveEvent (
    QEvent* e
) {
    emit mouseLeft ();
}
void AnchorLabel::mousePressEvent( QMouseEvent* ev )
{
    emit  mousePressed();
    emit  mousePressed(frameID);
}
void AnchorLabel::onMouseEnter () {
    frameLabel = new QLabel (this);
    frameLabel->setWindowFlags ( Qt::Window );

    std::string RES_IMG_PATH(Config::FramesPath());
    QPixmap pic(QString::fromUtf8(((RES_IMG_PATH + "f"+ toString(frameID) + "/texture.pgm").c_str())));
    frameLabel->setPixmap (
        pic    
    );
    frameLabel->show ();
    connect (
              this, SIGNAL (   mouseLeft () ),
        frameLabel,   SLOT (       close () )
    );
    connect (
        frameLabel, SIGNAL (   destroyed () ),
        frameLabel, SLOT   ( deleteLater () )
    );
}
void AnchorLabel::onMouseLeave () {
    frameLabel = (QLabel*)0x0;
}

AnchorLabel::AnchorLabel( const QString & text, QWidget * parent )
:QLabel(parent)
{
    oldFrameStyle = 16;
    connect ( 
        this, SIGNAL ( mousePressed () ), 
        this,   SLOT (  slotClicked () )
    );
    connect (
        this, SIGNAL ( mouseEntered () ),
        this,   SLOT ( onMouseEnter () )
    );
    connect (
        this, SIGNAL (    mouseLeft () ),
        this,   SLOT ( onMouseLeave () )
    );
}

AnchorLabel::AnchorLabel(QWidget * parent )
    :   QLabel(parent)
{
    connect ( 
        this, SIGNAL ( mousePressed () ), 
        this,   SLOT (  slotClicked () )
    );
    connect (
        this, SIGNAL ( mouseEntered () ),
        this,   SLOT ( onMouseEnter () )
    );
    connect (
        this, SIGNAL (    mouseLeft () ),
        this,   SLOT ( onMouseLeave () )
    );
}

void AnchorLabel::slotClicked()
{

    if(frameStyle() != 18){
        oldFrameStyle = frameStyle();
        this -> setFrameStyle(QFrame::Panel | QFrame::Plain);
        this -> setStyleSheet("color:red");
        this -> setLineWidth(3);
    }
    else if(oldFrameStyle == 17) {
        this -> setFrameStyle(QFrame::Box | QFrame::Plain);
        this -> setStyleSheet("color:blue");
        this -> setLineWidth(3);
    }
    else { 
        this -> setFrameStyle(oldFrameStyle);
    }
}

