/*  design.cc

    Mark Woolrich, Tim Behrens - FMRIB Image Analysis Group

    Copyright (C) 2002 University of Oxford  */

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

#include "design.h"
#include "utils/log.h"
#include "miscmaths/miscmaths.h"
#include "newmat.h"
#include "utils/tracer_plus.h"
#include "gsoptions.h"
#include <string>

using namespace Utilities;
using namespace MISCMATHS;
using namespace NEWMAT;

namespace Gs {

  void Design::setup(bool loadcontrasts)
  {
    Tracer_Plus trace("Design::setup");

    dm = read_vest(GsOptions::getInstance().designfile.value());
    //write_ascii_matrix(dm,"dm"); 
    nevs = dm.Ncols();
    ntpts = dm.Nrows();

    gind = read_vest(GsOptions::getInstance().covsplitfile.value());
    nevs = dm.Ncols();
    ntpts = dm.Nrows();
    ngs = int(gind.Maximum());

    if(nevs == 0 || ntpts == 0)
	throw Exception(string(GsOptions::getInstance().designfile.value() + " is an invalid design matrix file").data());

    if(ngs == 0 || gind.Nrows() == 0)
      throw Exception(string(GsOptions::getInstance().covsplitfile.value() + " is an invalid covariance split file").data());

    if(ntpts != gind.Nrows())
      {
	cerr << "design matrix has ntpts=" <<  ntpts << endl;
	cerr << "cov split file has ntpts=" << gind.Nrows() << endl;
	throw Exception("The ntpts needs to be the same for both");
      }

    // fill gind:
    covsplit.ReSize(ntpts,ngs);
    covsplit = 0;
    ntptsing.ReSize(ngs);
    ntptsing = 0;

    for(int t = 1; t <= ntpts; t++)
      {
	covsplit(t,int(gind(t)))=1;
	ntptsing(int(gind(t)))++;
      }

    cout << "ntptsing=" << ntptsing << endl;
//     cout << "gind=" << gind << endl;

    // assign each EV to a group
    evs_group.ReSize(nevs);
    evs_group=0;
    for(int e=1; e <=nevs; e++)
      {	  
	for(int t = 1; t <= ntpts; t++)
	  {	
	    if(dm(t,e)!=0)
	      {
		if(evs_group(e)==0)
		  {
		    evs_group(e) = gind(t);
		  }
		else if(evs_group(e)!=gind(t))
		  {
		    cerr << "nonseparable design matrix and variance groups" << endl;
		    throw Exception("nonseparable design matrix and variance groups");
		  }
	      }
	  }
      }

    //    OUT(evs_group);

    // load contrasts:
    if(loadcontrasts)
      {
	try
	  {
	    tcontrasts = read_vest(GsOptions::getInstance().tcontrastsfile.value());
	    numTcontrasts = tcontrasts.Nrows();	    
	  }
	catch(Exception exp)
	  {
	    numTcontrasts = 0;
	  }

	// Check contrast matches design matrix
	if(numTcontrasts > 0 && dm.Ncols() != tcontrasts.Ncols())
	  { 
	    cerr << "Num tcontrast cols  = " << tcontrasts.Ncols() << ", design matrix cols = " << dm.Ncols() << endl;
	    throw Exception("size of design matrix does not match t contrasts\n");
	  }

	try
	  {
	    fcontrasts = read_vest(GsOptions::getInstance().fcontrastsfile.value());
	    numFcontrasts = fcontrasts.Nrows();
	  }
	catch(Exception exp)
	  {
	    numFcontrasts = 0;
	    cout << "No f contrasts" << endl;
	  }

	if(numFcontrasts > 0)
	  {
	    // Check contrasts match
	    if(tcontrasts.Nrows() != fcontrasts.Ncols())
	      { 
		cerr << "tcontrasts.Nrows()  = " << tcontrasts.Nrows() << endl;
		cerr << "fcontrasts.Ncols()  = " << fcontrasts.Ncols() << endl;
		throw Exception("size of tcontrasts does not match fcontrasts\n");
	      }
	    
	    if(numFcontrasts > 0)
	      setupfcontrasts();
	  }    	
      }    
    
  }

  void Design::setupfcontrasts()
  {
    Tracer_Plus trace("Design::setupfcontrasts");
    
    fc.resize(numFcontrasts);
    //reduceddms.resize(numFcontrasts);

    for(int f=0; f < numFcontrasts; f++)
    {
      fc[f].ReSize(tcontrasts.Nrows(),nevs);
      fc[f] = 0;

      int count = 0;
    
      for(int c = 1; c <= tcontrasts.Nrows(); c++)
	{	
	  if(fcontrasts(f+1,c) == 1)
	    {	    
	      count++;
	      fc[f].Row(count) = tcontrasts.Row(c);	

	    }
	}
    
      fc[f] = fc[f].Rows(1,count);

//       const Matrix& tfc = fc[f]; 
      
//       reduceddms[f] = getdm()*(Identity(nevs)-tfc*pinv(tfc));
      
//       Matrix tmp = zeros(2,2);
//       tmp(1,1) = 1;
//       reduceddms[f] = getdm()*(Identity(nevs)-tmp);

//        OUT(f);
//        OUT(fc[f]);
//       OUT(fc[f].t()*fc[f].i()*fc[f].t());	
//       OUT(dm);
//        OUT(reduceddms[f]);


    }
  }

  
}

