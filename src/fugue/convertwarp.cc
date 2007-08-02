/*  convertwarp.cc

    Mark Jenkinson, FMRIB Image Analysis Group

    Copyright (C) 2001-2007 University of Oxford  */

/*  Part of FSL - FMRIB's Software Library
    http://www.fmrib.ox.ac.uk/fsl
    fsl@fmrib.ox.ac.uk
    
    Developed at FMRIB (Oxford Centre for Functional Magnetic Resonance
    Imaging of the Brain), Department of Clinical Neurology, Oxford
    University, Oxford, UK
    
    
    LICENCE
    
    FMRIB Software Library, Release 4.0 (c) 2007, The University of
    Oxford (the "Software")
    
    The Software remains the property of the University of Oxford ("the
    University").
    
    The Software is distributed "AS IS" under this Licence solely for
    non-commercial use in the hope that it will be useful, but in order
    that the University as a charitable foundation protects its assets for
    the benefit of its educational and research purposes, the University
    makes clear that no condition is made or to be implied, nor is any
    warranty given or to be implied, as to the accuracy of the Software,
    or that it will be suitable for any particular purpose or for use
    under any specific conditions. Furthermore, the University disclaims
    all responsibility for the use which is made of the Software. It
    further disclaims any liability for the outcomes arising from using
    the Software.
    
    The Licensee agrees to indemnify the University and hold the
    University harmless from and against any and all claims, damages and
    liabilities asserted by third parties (including claims for
    negligence) which arise directly or indirectly from the use of the
    Software or the sale of any products based on the Software.
    
    No part of the Software may be reproduced, modified, transmitted or
    transferred in any form or by any means, electronic or mechanical,
    without the express permission of the University. The permission of
    the University is not required if the said reproduction, modification,
    transmission or transference is done without financial return, the
    conditions of this Licence are imposed upon the receiver of the
    product, and all original and amended source code is included in any
    transmitted product. You may be held legally responsible for any
    copyright infringement that is caused or encouraged by your failure to
    abide by these terms and conditions.
    
    You are not permitted under this Licence to use this Software
    commercially. Use for which any financial return is received shall be
    defined as commercial use, and includes (1) integration of all or part
    of the source code or the Software into a product for sale or license
    by or on behalf of Licensee to third parties or (2) use of the
    Software or any derivative of it for research with the final aim of
    developing software products for sale or license to a third party or
    (3) use of the Software or any derivative of it for research with the
    final aim of developing non-software products for sale or license to a
    third party, or (4) use of the Software to provide any service to an
    external organisation for which payment is received. If you are
    interested in using the Software commercially, please contact Isis
    Innovation Limited ("Isis"), the technology transfer company of the
    University, to negotiate a licence. Contact details are:
    innovation@isis.ox.ac.uk quoting reference DE/1112. */

#include "utils/options.h"
#include "newimage/warpfns.h"

#define _GNU_SOURCE 1
#define POSIX_SOURCE 1

using namespace Utilities;
using namespace NEWIMAGE;

////////////////////////////////////////////////////////////////////////////

// COMMAND LINE OPTIONS

string title="convertwarp (Version 2.0)\nCopyright(c) 2001-2007, University of Oxford (Mark Jenkinson)";
string examples="convertwarp -m affine_matrix_file -r refvol -o output_warp\nconvertwarp --premat=mat1 --warp1=vol1 --warp2=vol2 --postmat=mat2 -o output_warp\nconvertwarp -s shiftmapvol -o output_warp";

Option<bool> verbose(string("-v,--verbose"), false, 
		     string("switch on diagnostic messages"), 
		     false, no_argument);
Option<bool> help(string("-h,--help"), false,
		  string("display this message"),
		  false, no_argument);
Option<bool> abswarp(string("--abs"), false,
		  string("use absolute warp convention (default): x' = w(x)"),
		  false, no_argument);
Option<bool> relwarp(string("--rel"), false,
		  string("use relative warp convention: x' = x + w(x)"),
		  false, no_argument);
Option<bool> relwarpout(string("--relout"), false,
			string("force output to use relative warp convention: x' = x + w(x)"),
		  false, no_argument);
Option<bool> abswarpout(string("--absout"), false,
		  string("force output to use absolute warp convention: x' = w(x)"),
		  false, no_argument);
Option<bool> jacobianstats(string("--jstats"), false,
		  string("print out statistics of the Jacobian of the warpfield"),
		  false, no_argument);
Option<bool> constrainj(string("--constrainj"), false,
		  string("constrain the Jacobian of the warpfield to lie within specified min/max limits"),
		  false, no_argument);
Option<float> jmin(string("--jmin"), 0.01,
			string("minimum acceptable Jacobian value for constraint (default 0.01)"),
			false, requires_argument);
Option<float> jmax(string("--jmax"), 100.0,
			string("maximum acceptable Jacobian value for constraint (default 100.0)"),
			false, requires_argument);
Option<string> prematname(string("-m,--premat"), string(""),
			  string("filename of pre-affine transform (not MEDx)"),
			  false, requires_argument);
Option<string> midmatname(string("--midmat"), string(""),
			  string("filename of mid-warp-affine transform (not MEDx)"),
			  false, requires_argument);
Option<string> postmatname(string("--postmat"), string(""),
			  string("filename of post-affine transform (not MEDx)"),
			  false, requires_argument);
Option<string> shiftmapname(string("-s,--shiftmap"), string(""),
		       string("filename for shiftmap (applied last)"),
		       false, requires_argument);
Option<string> warp1name(string("-w,--warp1"), string(""),
		       string("filename for initial warp (follows pre-affine)"),
		       false, requires_argument);
Option<string> warp2name(string("--warp2"), string(""),
		       string("filename for secondary warp (after initial warp, before post-affine)"),
		       false, requires_argument);
Option<string> refname(string("-r,--ref"), string(""),
		       string("filename for reference image"),
		       false, requires_argument);
Option<string> outname(string("-o,--out"), string(""),
		       string("filename for output (warp) image"),
		       false, requires_argument);
Option<string> shiftdir(string("-d,--shiftdir"), string("y"),
		       string("direction to apply shiftmap {x,y,z,x-,y-,z-}"),
		       false, requires_argument);
Option<string> jacobianname(string("-j,--jacobian"), string("y"),
		       string("calculate and save Jacobian of final warp field"),
		       false, requires_argument);

bool abs_warp = true;


////////////////////////////////////////////////////////////////////////////

void update_warp(volume4D<float>& totalwarp,  const volume4D<float>& newwarp,
		 bool& warpset)
{
  if (warpset) { 
    volume4D<float> tmpwarp = totalwarp;  // previous warp (prewarp)
    concat_warps(tmpwarp,newwarp,totalwarp);
  } else {
    totalwarp = newwarp; 
    warpset = true;
  }
}


bool getabswarp(volume4D<float>& warpvol) {
  bool absw = false;
  if (abswarp.set()) { absw = true; }
  else if (relwarp.set()) { absw = false; }
  else {
    if (verbose.value()) { 
      cout << "Automatically determining relative/absolute warp conventions" << endl; 
    }
    absw = is_abs_convention(warpvol);
    if (verbose.value()) {
      if (absw) { cout << "Warp convention = absolute" << endl; } 
      else { cout << "Warp convention = relative" << endl; }
    }
  }
  return absw;
}


int convert_warp()
{
  volume<float> refvol;

  if (refname.set()) {
    read_rad_volume(refvol,refname.value());
  } else if (warp2name.set()) {
    read_rad_volume(refvol,warp2name.value());  // only reads first volume
  } else if (warp1name.set()) {
    read_rad_volume(refvol,warp1name.value());  // only reads first volume
  } else if (shiftmapname.set()) {
    read_rad_volume(refvol,shiftmapname.value());  // only reads first volume
  } else {
    cerr << "Cannot determine size of output warp!" << endl;
    cerr << "Must specify a reference volume (using -r)" << endl;
    exit(EXIT_FAILURE);
  }

  volume4D<float> nextwarp, finalwarp;
  bool warpset = false;

  // Note that internally everything works with absolute warps

  Matrix premat, midmat, postmat;
  premat = Identity(4);
  midmat = Identity(4);
  postmat = Identity(4);

  if (prematname.set()) {
    read_ascii_matrix(premat,prematname.value());
    affine2warp(premat,nextwarp,refvol);
    update_warp(finalwarp,nextwarp,warpset);
  }

  if (warp1name.set()) {
    read_rad_volume4D(nextwarp,warp1name.value());
    abs_warp = getabswarp(nextwarp);
    if (!abs_warp) { convertwarp_rel2abs(nextwarp); }
    update_warp(finalwarp,nextwarp,warpset);
  }

  if (midmatname.set()) {
    read_ascii_matrix(midmat,midmatname.value());
    affine2warp(midmat,nextwarp,refvol);
    update_warp(finalwarp,nextwarp,warpset);
  }

  if (warp2name.set()) {
    read_rad_volume4D(nextwarp,warp2name.value());
    abs_warp = getabswarp(nextwarp);
    if (!abs_warp) { convertwarp_rel2abs(nextwarp); }
    update_warp(finalwarp,nextwarp,warpset);
  }

  if (postmatname.set()) {
    read_ascii_matrix(postmat,postmatname.value());
    affine2warp(postmat,nextwarp,refvol);
    update_warp(finalwarp,nextwarp,warpset);
  }

  // apply shiftmap last (if it exists)
  if (shiftmapname.set()) {
    volume<float> shiftmap;
    read_rad_volume(shiftmap,shiftmapname.value());
    shift2warp(shiftmap,nextwarp,shiftdir.value());
    update_warp(finalwarp,nextwarp,warpset);
  }

  // force the final warp to be the same size as refvol (if it isn't already)
  if (!samesize(finalwarp[0],refvol)) {
    affine2warp(Identity(4),nextwarp,refvol);
    update_warp(finalwarp,nextwarp,warpset);
  }
   
  if (jacobianstats.value() || jacobianname.set()) {
    ColumnVector jstats;
    if (jacobianname.set()) {
      volume4D<float> jvol;
      jvol=jacobian_check(jstats,finalwarp,0.01,100.0);
      save_volume4D(jvol,jacobianname.value());
    }
    if (jacobianstats.value()) {
      if (jacobianname.unset()) { jstats=jacobian_quick_check(finalwarp,jmin.value(),jmax.value()); }
      cout << "Jacobian : min, max = "<<jstats(1)<<", "<<jstats(2)<<endl;
      cout << "Number of voxels where J < " << jmin.value() << " = "<<MISCMATHS::round(jstats(3)/8.0)<<endl;
      cout << "Number of voxels where J > " << jmax.value() << " = "<<MISCMATHS::round(jstats(4)/8.0)<<endl;
    }
  }

  if (constrainj.value()) {
    constrain_topology(finalwarp,jmin.value(),jmax.value());
  }

  if (relwarpout.set() || (abswarpout.unset() && !abs_warp)) {
    // convert output to relative
    convertwarp_abs2rel(finalwarp);
  }

  if (outname.set()) {
    save_volume4D(finalwarp,outname.value());
  }
  
  return 0;
}





int main(int argc, char* argv[])
{
  Tracer tr("main");

  OptionParser options(title, examples);

  try {
    options.add(outname);
    options.add(prematname);
    options.add(warp1name);
    options.add(midmatname);
    options.add(warp2name);
    options.add(postmatname);
    options.add(refname);
    options.add(shiftmapname);
    options.add(shiftdir);
    options.add(jacobianname);
    options.add(jacobianstats);
    options.add(constrainj);
    options.add(jmin);
    options.add(jmax);
    options.add(abswarp);
    options.add(relwarp);
    options.add(abswarpout);
    options.add(relwarpout);
    options.add(verbose);
    options.add(help);
    
    options.parse_command_line(argc, argv);

    if ( (argc<2) || (help.value()) || (!options.check_compulsory_arguments(true)) )
      {
	options.usage();
	exit(EXIT_FAILURE);
      }
  }  catch(X_OptionError& e) {
    options.usage();
    cerr << endl << e.what() << endl;
    exit(EXIT_FAILURE);
  } catch(std::exception &e) {
    cerr << e.what() << endl;
  } 

  return convert_warp();
}




