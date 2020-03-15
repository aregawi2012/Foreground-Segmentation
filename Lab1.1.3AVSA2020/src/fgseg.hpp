/* Applied Video Sequence Analysis (AVSA)
 *
 *	LAB1.0: Background Subtraction - Unix version
 *	fgesg.hpp
 *
 * 	Authors: José M. Martínez (josem.martinez@uam.es), Paula Moral (paula.moral@uam.es) & Juan Carlos San Miguel (juancarlos.sanmiguel@uam.es)
 *	VPULab-UAM 2020
 */


#include <opencv2/opencv.hpp>

#ifndef FGSEG_H_INCLUDE
#define FGSEG_H_INCLUDE

using namespace cv;
using namespace std;

namespace fgseg {


	//Declaration of FGSeg class based on BackGround Subtraction (bgs)
	class bgs{
	public:

		/*  ***************************************************
		 *
		 *  CONSTRUCTORS
		 *
		 *  *************************************************
		 */

		//constructor with parameter "threshold"
		bgs(double threshold, bool rgb);

		// constructor with alpha and selective update
        bgs(double threshold, double alpha, bool selective_bkg_update , bool rgb);

        //Constructor for with gost2
        bgs(double tau, double alpha, bool selective_bkg_update, int threshold_ghosts2, bool rgb);
		//destructor
		~bgs(void);


		/*  ***************************************************
		 *
		 *  HELPER FUNCTIONS
		 *
		 *  *************************************************
		 */


		// method to initialize bkg (first frame - hot start)
		void init_bkg(cv::Mat Frame);

		// method to perform BackGroundSubtraction
		void bkgSubtraction(cv::Mat Frame);

		//method to detect and remove shadows in the binary BGS mask
		void removeShadows();

		// Running  average
		void running_average();

		// suppression of Stationary Objects
		void suppression_stationary();


		/*  ***************************************************
		 *
         *  GETTERS
         *
         *  ***************************************************
         */


		//returns the BG image
		cv::Mat getBG(){return _bkg;};

		//returns the DIFF image
		cv::Mat getDiff(){return _diff;};

		//returns the BGS mask
		cv::Mat getBGSmask(){return _bgsmask;};

		//returns the binary mask with detected shadows
		cv::Mat getShadowMask(){return _shadowmask;};

		//returns the binary FG mask
		cv::Mat getFGmask(){return _fgmask;};


		//ADD ADITIONAL METHODS HERE
		//...
	private:
		cv::Mat _bkg; //Background model
		cv::Mat	_frame; //current frame
		cv::Mat _diff; //abs diff frame
		cv::Mat _bgsmask; //binary image for bgssub (FG)
		cv::Mat _shadowmask; //binary mask for detected shadows
		cv::Mat _fgmask; //binary image for foreground (FG)


		bool _rgb;
		double _threshold;
		//ADD ADITIONAL VARIABLES HERE

		bool _selective_bkg_update;
        double _alpha;
        int _threshold_ghosts2;


        // Variable for  color images
		cv::Mat _diff_channels[3],_frame_channels[3],_bkg_channels[3],_bgsmask_channels[3]; // for the splits

		// For Morphological operation
		 cv::Mat element = getStructuringElement(MORPH_CROSS, Size( 2*2 + 1, 2*2+1 ), Point( 2, 2 ) );

		// logical masks of foreground and background for matrix operation
		cv::Mat bg_logical_mask , fg_logical_mask;

		// pixel counter
		cv::Mat _pixel_counter;




	};//end of class bgs

}//end of namespace

#endif




