/*  fwdmodel_custom.h - A place for your own quick'n'dirty forward model implementations.

    Adrian Groves, FMRIB Image Analysis Group

    Copyright (C) 2008 University of Oxford  */

/*  Part of FSL - FMRIB's Software Library
    http://www.fmrib.ox.ac.uk/fsl
    fsl@fmrib.ox.ac.uk
    
    Developed at FMRIB (Oxford Centre for Functional Magnetic Resonance
    Imaging of the Brain), Department of Clinical Neurology, Oxford
    University, Oxford, UK
    
    
    LICENCE
    
    FMRIB Software Library, Release 5.0 (c) 2012, The University of
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
    innovation@isis.ox.ac.uk quoting reference DE/9564. */


/* fwdmodel_custom.h
 * Class declaration for a custom forward model
 * Template written by Adrian Groves, 2008
 * FMRIB Centre, University of Oxford
 *
 * Last modified: $Date: 2009/01/23 12:58:53 $ $Author: adriang $ $Revision: 1.1 $
 */

#ifndef __FABBER_FWDMODEL_CUSTOM_H
#define __FABBER_FWDMODEL_CUSTOM_H 1

#include "fwdmodel.h"

using namespace NEWMAT;
using namespace std;

/* Ideally you should create your own fwdmodel_*.h and .cc files to implement your models.
 * This "custom" model gives you a place to put your code and quickly start fitting your model.
 */

class CustomFwdModel : public FwdModel {
 public:
  CustomFwdModel(ArgsType& args);
  virtual ~CustomFwdModel() { return; } 	

  virtual void Evaluate(const ColumnVector& params, ColumnVector& result) const;
  virtual string ModelVersion() const;
  virtual int NumParams() const;
  virtual void HardcodedInitialDists(MVNDist& prior, MVNDist& posterior) const;
  virtual void NameParams(vector<string>& names) const;
  // use default implementation of DumpParameters

  static void ModelUsage();

 protected:
  // TODO: put your implementation-specific variables here.  These include any constants or
  // flags that are read from the command line.

  // int numRepeats;          // e.g. number of repeats
  // double T1;               // e.g. a fixed scan parameter
  // ColumnVector echoTimes;  // e.g. a list of TEs
  // Matrix basis;            // e.g. a basis set
  // bool wantFriesWithThat;  // e.g. a boolean option
};

#endif // __FABBER_FWDMODEL_CUSTOM_H
