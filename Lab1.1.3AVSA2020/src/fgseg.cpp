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
bgs::bgs(double threshold, double alpha,bool selective_bkg_update,int threshold_ghosts2,bool rgb){
	_rgb = rgb ;
    _threshold = threshold;
    _selective_bkg_update = selective_bkg_update;
    _alpha  = alpha;
    _threshold_ghosts2 = threshold_ghosts2;

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
	  	 _pixel_counter = Mat::zeros(Size(Frame.cols,Frame.rows), CV_8UC1);

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
    // init Shadow Mask (currently Shadow Detection not implemented)
    _bgsmask.copyTo(_shadowmask); // creates the mask (currently with bgs)

    //ADD YOUR CODE HERE


    //...
    absdiff(_bgsmask, _bgsmask, _shadowmask);// currently void function mask=0 (should create shadow mask)
    //...

    absdiff(_bgsmask, _shadowmask, _fgmask); // eliminates shadows from bgsmask
}

// selective running average for model update
void bgs::running_average(){

 /**
  *
  * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  *  Function : running_average
  *  input   :  alpha
  *  Output  : _bkg =>  Update background  model
  * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  *
  */

	 // General Masks which will be used in all the cases
	 // Logical Masks used for optimal calculation of new updated background
	 fg_logical_mask = _bgsmask / 255;
	 bg_logical_mask  = (1- fg_logical_mask);


	 // GRAY SCALE IMAGES ONLY
	if(!_rgb){

		 if(_selective_bkg_update){

			   // temporary background which will be updated using the logical masks if pixel belongs to background
			   Mat temp_bkg = _alpha*_frame + (1-_alpha)*_bkg;

			   // Update background if _bkmask == 0
			   _bkg = fg_logical_mask.mul(_bkg) + bg_logical_mask.mul(temp_bkg);

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
		vector<Mat> accumlator ;  // for merging back the results of individual channels
        Mat copy_accomulator ;    // for copying the content of the vector
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
        		 // Calculate the possible update
        		 temp_bkg[i] = _alpha*_frame_channels[i] + (1-_alpha)*_bkg_channels[i];
                 accumlator.push_back(temp_bkg[i]);
        	 }

    	 }

         // change the vector into matrix by merging
         merge(accumlator , copy_accomulator);

		 // Finally whatever the accumlator has copy to _bkg :). Cool
         copy_accomulator.copyTo(_bkg);

       }


}


// Superposision of stationary object
void bgs::suppression_stationary(){


	 // General Masks which will be used in all the cases
	 // Logical Masks used for optimal calcualtion of new updated background
	 fg_logical_mask = _bgsmask / 255;
	 bg_logical_mask  = (1-fg_logical_mask);

	 // Reset all the counter for all the pixels that are part of the background (only foreground will be 1)
	 _pixel_counter = _pixel_counter.mul(fg_logical_mask);

	 // Increment counter  if pixel is foreground // only foreground
	 _pixel_counter =_pixel_counter + fg_logical_mask;

	 // check if pixel exceeds from threshold or not .. create the logical mask
	 Mat pixel_exceeds_threshold = (_pixel_counter >= _threshold_ghosts2) / 255; // logical : 1 : exceed , 0:no
	 Mat pixel_less_threshold =    1-pixel_exceeds_threshold ;


	 //Get part of the image/frame through with their threshold is exceeded ,,, we want to incorporate them
	 Mat incorporate_pixel = pixel_exceeds_threshold.mul(_frame);

	 // Get the parts of background that are not going to be updated
     Mat bkg_no_update  = pixel_less_threshold.mul(_bkg);


     // Now get the updated background
    _bkg = bkg_no_update +incorporate_pixel;

     // finally update the _bkg with the new data
     Mat new_bkg = _alpha*_frame + (1-_alpha)*_bkg;
    _bkg = fg_logical_mask.mul(_frame) + bg_logical_mask.mul(new_bkg);
    /// MORPHOLOGICAL CLOSING APPLIED BELOW .. FOR CLOSING .. DOESN'T TURN OUT GOOD


}

