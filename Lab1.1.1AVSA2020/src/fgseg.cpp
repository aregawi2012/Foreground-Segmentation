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
#include <iostream>

using namespace std;
using namespace fgseg;


/*  ***************************************************
 *
 *  CONSTRUCTORS and DESTRUCTOR
 *
 *  *************************************************
 */

bgs::bgs(double threshold, bool rgb)
{
	_rgb=rgb;
	_threshold=threshold;
}


//default
bgs::~bgs(void)
{
}

//method to initialize bkg (first frame - hot start)
void bgs::init_bkg(cv::Mat Frame)
{

	if (!_rgb){

		cvtColor(Frame, Frame, COLOR_BGR2GRAY);                 // to work with gray even if input is color
       _bkg = Mat::zeros(Size(Frame.cols,Frame.rows), CV_8UC1); // void function for Lab1.0 - returns zero matrix
	}

	else{
	       _bkg = Mat::zeros(Size(Frame.cols,Frame.rows), CV_8UC3); // void function for Lab1.0 - returns zero matrix
	}


	Frame.copyTo(_bkg);

}


//method to perform BackGroundSubtraction
void bgs::bkgSubtraction(cv::Mat Frame)
{

	/*
	 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	 *  Input : Frame , *threshold
	 *  Output : _bgsmask -> 1/255 - foreground
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

         /// Apply the erosion operation
         //  dilate( this-> _bgsmask, this-> _bgsmask, element );
         //  erode( this-> _bgsmask, this-> _bgsmask, element );

	}
	else{


		  Frame.copyTo(this->_frame);

         // GENERAL difference between the 3 channel frame and 3 channel background model
		 absdiff(this->_frame ,this-> _bkg ,this-> _diff);

         // split the difference and threshold each channel
         split(this->_diff , this->_split_channels);

        // threshold every split
        threshold(this->_split_channels[0],this-> _bgsmask_channels[0], this->_threshold, 255, cv::THRESH_BINARY); // 	BLUE
        threshold(this->_split_channels[1],this-> _bgsmask_channels[1], this->_threshold, 255, cv::THRESH_BINARY); //  GREEN
        threshold(this->_split_channels[2],this-> _bgsmask_channels[2], this->_threshold, 255, cv::THRESH_BINARY); //  RED

        // combine every split using bitwise_or and add them to _bgmask
        bitwise_or(this->_bgsmask_channels[0],this->_bgsmask_channels[1],this->_bgsmask);
        bitwise_or(this->_bgsmask_channels[3],this->_bgsmask, this->_bgsmask);

	  }

}

//method to detect and remove shadows in the BGS mask to create FG mask
void bgs::removeShadows()
{
    // init Shadow Mask (currently Shadow Detection not implemented)
    _bgsmask.copyTo(_shadowmask); // creates the mask (currently with bgs)

    /*
     *  ACTUAL IMPELEMENTATION COMMING SOON ON TASK 3.1
     */



    //...
    absdiff(_bgsmask, _bgsmask, _shadowmask);// currently void function mask=0 (should create shadow mask)

    absdiff(_bgsmask, _shadowmask, _fgmask); // eliminates shadows from bgsmask
}


