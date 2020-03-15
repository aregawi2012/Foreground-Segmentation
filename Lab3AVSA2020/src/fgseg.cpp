/* Applied Video Sequence Analysis (AVSA)
 *
 *	LAB1.0: Background Subtraction - Unix version
 *	fgesg.cpp
 *
 * 	Authors: José M. Martínez (josem.martinez@uam.es), Paula Moral (paula.moral@uam.es) & Juan Carlos San Miguel (juancarlos.sanmiguel@uam.es)
 *	VPULab-UAM 2020
 */

#include <opencv2/opencv.hpp>
#include "fgseg.hpp"

using namespace fgseg;


// Lab 1.2 constructor - selective background update
bgs::bgs(double threshold, double alpha,bool selective_bkg_update,int threshold_ghosts2,bool rgb , double alpha2 , double beta , double ts , double th){
	_rgb = rgb ;
    _threshold = threshold;
    _selective_bkg_update = selective_bkg_update;
    _alpha  = alpha;
    _threshold_ghosts2 = threshold_ghosts2;
    _alpha2 = alpha2;
    _beta = beta;
    _ts = ts ;
    _th = th ;

}


//default destructor
bgs::~bgs(void)
{
}



//method to initialize bkg (first frame - hot start)
void bgs::init_bkg(cv::Mat Frame)
{

	  if (!_rgb){

			cvtColor(Frame, Frame, COLOR_BGR2GRAY); // to work with gray even if input is color
	       _bkg = Mat::zeros(Size(Frame.cols,Frame.rows), CV_8UC1); // void function for Lab1.0 - returns zero matrix

		}

		Frame.copyTo(_bkg);
	   _pixel_counter = Mat::zeros(Size(Frame.cols,Frame.rows), CV_8UC1);
	   // Initialize counter to zeros


}

//method to perform BackGroundSubtraction
void bgs::bkgSubtraction(cv::Mat Frame)
{

	    /*
		 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		 *  Input : Frame ,
		 *  Output : _bgsmask -> 1/255 - forgroud
		 *                    -> 0     - background
		 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		 *
		 */
	if (!_rgb){

		_bgsmask = Mat::zeros(Size(Frame.cols,Frame.rows), CV_8UC1); // void function for Lab1.0 - returns zero matrix

		// Intialize and convert to gray scale and copy
		 _diff = Mat::zeros(Size(Frame.cols,Frame.rows), CV_8UC1);
		 cvtColor(Frame, Frame, COLOR_BGR2GRAY);
	     Frame.copyTo(_frame);

		 // calculate the difference between current frame and bgmodel
		 absdiff(Frame,this->getBG(),this->_diff);

		 // Threshold difference
         threshold(this->_diff,this-> _bgsmask, this->_threshold, 255, cv::THRESH_BINARY);


          /// MORPHOLOGICAL CLOSING APPLIED BELOW .. FOR CLOSING .. DOESN'T TURN OUT GOOD

            // Apply the erosion operation
            //  dilate( this-> _bgsmask, this-> _bgsmask, element );
           // erode( this-> _bgsmask, this-> _bgsmask, element );

	}
	else{


		  Frame.copyTo(this->_frame);

         // GENERAL difference between the 3 channel frame and 3 channel background model
		 absdiff(this->_frame ,this-> _bkg ,this-> _diff);

         // split the difference and threshold each channel
         split(this->_diff , this->_diff_channels);
		 cvtColor(this->_diff, this->_diff, COLOR_BGR2GRAY);

        // threshold every split
        threshold(this->_diff_channels[0],this-> _bgsmask_channels[0], this->_threshold, 255, cv::THRESH_BINARY); // 	BLUE
        threshold(this->_diff_channels[1],this-> _bgsmask_channels[1], this->_threshold, 255, cv::THRESH_BINARY); //  GREEN
        threshold(this->_diff_channels[2],this-> _bgsmask_channels[2], this->_threshold, 255, cv::THRESH_BINARY); //  RED

        // combine every split using bitwise_or and add them to _bgmask
        bitwise_or(this->_bgsmask_channels[0],this->_bgsmask_channels[1],this->_bgsmask);
        bitwise_or(this->_bgsmask_channels[2],this->_bgsmask, this->_bgsmask);

	  }


}

//method to detect and remove shadows in the BGS mask to create FG mask
void bgs::removeShadows()
{

    /**
     *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *    Shadow Elimination using chonomatic
     *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     */

      // init Shadow Mask (currently Shadow Detection not implemented
     _shadowmask = Mat::zeros(Size(_bgsmask.cols,_bgsmask.rows), CV_8UC1);
     _fgmask = Mat::zeros(Size(_bgsmask.cols,_bgsmask.rows), CV_8UC1);


       // convert BGR TO HSV
       cvtColor(_frame, frame_hsv, CV_BGR2HSV);
       cvtColor(_bkg, bkg_hsv, CV_BGR2HSV);

       // Split both Frame and background
       vector<Mat> frame_hsv_channels , bkg_hsv_channels;
       split(frame_hsv, frame_hsv_channels);
       split(bkg_hsv, bkg_hsv_channels);

       // Calculate the different conditions
         bool condition1 , condition2 , condition3;
         double DH  ;

         // check every pixel
		   int nRows = _frame.rows;
		   int nCols = _frame.cols;
		   int i , j;

		   for(i = 0 ; i < nRows ; i++){

				for(j= 0 ; j < nCols;j++){




				      condition1 = (_alpha2 <=  frame_hsv_channels[2].at<double>(i,j) / bkg_hsv_channels[2].at<double>(i,j)) && frame_hsv_channels[2].at<double>(i,j) / bkg_hsv_channels[2].at<double>(i,j) <=_beta;

				      condition2 = abs(frame_hsv_channels[1].at<double>(i,j) - bkg_hsv_channels[1].at<double>(i,j)) <= _ts ;

				      double term1 = abs(frame_hsv_channels[0].at<double>(i,j)- bkg_hsv_channels[0].at<double>(i,j));
		              double term2 = 360 - term1;
		              if(term1 < term2){
		            	  DH = term1;
		              }else{
		            	  DH = term2;
		              }

		              condition3 = DH < _th ;

		              if(condition1 && condition2 && condition3){

		             	 _shadowmask.at<uchar>(i,j) = 255;
		             	// _fgmask.at<uchar>(i,j) = _bgsmask.at<uchar>(i,j)  -  _shadowmask.at<uchar>(i,j);
		              }

					}

			   }

		    absdiff(_bgsmask, _shadowmask, _fgmask); // eliminates shadows from bgsmask
		}

// selective running average for model update
void bgs::running_average(){

 /**
  *
  * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  *  Function : Running Average
  *  input   : ~
  *  Output  : _bkg ,  Update background  model
  * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  *
  */


	 // General Masks which will be used in all the cases
	 // Logical Masks used for optimal calculation of new updated background
	 fg_logical_mask = _bgsmask / 255;
	 bg_logical_mask  = 1- fg_logical_mask;


	  // GRAY SCALE IMAGES ONLY
	  if(!_rgb){

		 if(_selective_bkg_update){


			 int nRows = _frame.rows;
			 int nCols = _frame.cols;
			 int i , j;

			 for(i = 0 ; i < nRows ; i++){
				for(j= 0 ; j < nCols;j++){

					   if(_bgsmask.at<uchar>(i,j) == 0 ){

						   _pixel_counter.at<uchar>(i,j) = 0 ;
						   _bkg.at<uchar>(i,j) =_alpha*_frame.at<uchar>(i,j) + (1-_alpha)*_bkg.at<uchar>(i,j);

					   }else{

						   _pixel_counter.at<uchar>(i,j) = _pixel_counter.at<uchar>(i,j)+ 1;

					   }

				   }
			   }

		 }else {

			 // blind update
			 _bkg = _alpha*_frame + (1-_alpha)*_bkg;

		 }
    }

	//  COLOURED IMAGES
	else{


        // Split channels , Both for blind and selective update
		split(_frame,_frame_channels);
		split(_bkg , _bkg_channels);

		// temporary bkg holder which will be altered by logical mask
		Mat temp_bkg[3];
		vector<Mat> accumlator ; // for merging back the results of individual channels
        Mat update ;


		 if(_selective_bkg_update){


			 // for each three channels
			 for ( int i = 0 ; i< 3 ; i ++){

				    // Calculate the possible update , regardless , it will be filtered by the logical mask
                	temp_bkg[i] = _alpha*_frame_channels[i] + (1-_alpha)*_bkg_channels[i];
                    temp_bkg[i] = fg_logical_mask.mul(_bkg_channels[i]) + bg_logical_mask.mul(temp_bkg[i]);
                    accumlator.push_back(temp_bkg[i]);

                }

         }else {

        	 // for each channel blindly update them

        	 for ( int i = 0 ; i< 3 ; i ++){
        		 // Calculate the possible update , regardless , it will be filtered by the logical mask
        		temp_bkg[i] = _alpha*_frame_channels[i] + (1-_alpha)*_bkg_channels[i];
                accumlator.push_back(temp_bkg[i]);
        	 }

    	 }

         merge(accumlator , update);

		 // Finally whatever the accumlator has copy to _bkg :). Cool
        update.copyTo(_bkg);

       }


}

void bgs::suppression_stationary(){


		 int nRows = _frame.rows;
		 int nCols = _frame.cols;
		 int i , j;

		 for(i = 0 ; i < nRows ; i++){
			for(j= 0 ; j < nCols;j++){

				   if(_pixel_counter.at<uchar>(i,j) >=_threshold_ghosts2 ){
					   _bkg.at<uchar>(i,j) = _frame.at<uchar>(i,j);

				   }

			   }
		   }

}

