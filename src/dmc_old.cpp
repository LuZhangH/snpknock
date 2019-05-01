/*
  This file is part of SNPknock.

    Copyright (C) 2017 Matteo Sesia

    SNPknock is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SNPknock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SNPknock.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef O_KNOCKOFF_DMC_CPP
#define O_KNOCKOFF_DMC_CPP

#include "dmc_old.h"

oKnockoffDMC::oKnockoffDMC(const std::vector<double> & _initP, const std::vector< matrix > & _Q, int seed){
  initP = _initP;
  Q = _Q;
  p = Q.size()+1;
  K = initP.size();

  Z = std::vector<double> (K);
  Z_old = std::vector<double> (K);
  W = std::vector<double> (K);
  Xt = std::vector<int> (p);

  gen = std::mt19937();
  gen.seed(seed);
  dis = std::uniform_real_distribution<double>(0.0,1.0);
}

oKnockoffDMC::~oKnockoffDMC(){}

std::vector<int> oKnockoffDMC::sample(const std::vector<int> & X) {
  std::fill(Z.begin(), Z.end(), 0.0);  // Reset Z to zeros
  for(int u=0; u<K; u++) {
    for(int k=0; k<K; k++) {
      Z[k] += initP[u] * Q[0][u][k];
    }
  }
  for(int k=0; k<K; k++) {
    W[k] = initP[k] * Q[0][k][X[1]] / Z[X[1]];
    Z_old[k] = Z[k];
  }  
  Xt[0] = weighted_choice(dis(gen),W);

  // // DEBUG
  // std::cout << "Partition function number " << 0 << ": " << std::endl << "\t";
  // for(int k=0; k<K; k++) {
  //   std::cout << Z[k] << " ";
  // }
  // std::cout<<std::endl;
  // std::cout << "Weights for element " << 0 << ": " << std::endl << "\t";
  // for(int k=0; k<K; k++) {
  //   std::cout << W[k] << " ";
  // }
  // std::cout<<std::endl;

  // Create the central p-2 knockoff
  for(int j=1; j<p-1; j++){
    std::fill(Z.begin(), Z.end(), 0.0);  // Reset Z to zeros
    for(int u=0; u<K; u++) {
      tempQ = Q[j-1][X[j-1]][u] * Q[j-1][Xt[j-1]][u] / Z_old[u];
      for(int k=0; k<K; k++) {
        Z[k] += tempQ*Q[j][u][k];
      }
    }
    for(int k=0; k<K; k++) {
      W[k] = Q[j-1][X[j-1]][k] * Q[j-1][Xt[j-1]][k] * Q[j][k][X[j+1]] / (Z_old[k] * Z[X[j+1]]);
      Z_old[k] = Z[k];      
    }
    Xt[j] = weighted_choice(dis(gen),W);

    // // DEBUG
    // std::cout << "Partition function number " << j << ": " << std::endl << "\t";
    // for(int k=0; k<K; k++) {
    //   std::cout << Z[k] << " ";
    // }
    // std::cout<<std::endl;
    // std::cout << "Weights for element " << j << ": " << std::endl << "\t";
    // for(int k=0; k<K; k++) {
    //   std::cout << W[k] << " ";
    // }
    // std::cout<<std::endl;
  }

  // Create the last knockoff
  double Zp = 0.0;
  for(int u=0; u<K; u++) {
    Zp += Q[p-2][X[p-2]][u] * Q[p-2][Xt[p-2]][u] / Z_old[u];
  }
  for(int k=0; k<K; k++) {
    W[k] = Q[p-2][X[p-2]][k] * Q[p-2][Xt[p-2]][k] / (Z_old[k] * Zp);
  }
  Xt[p-1] = weighted_choice(dis(gen),W);

  // // DEBUG
  // std::cout << "Weights for element " << p-1 << ": " << std::endl << "\t";
  // for(int k=0; k<K; k++) {
  //   std::cout << W[k] << " ";
  // }
  // std::cout<<std::endl;

  return(Xt);
}

std::vector< std::vector<int> > oKnockoffDMC::sample(const std::vector<std::vector<int> > & X) {
  int n = X.size();
  std::vector< std::vector<int> > XtMat(n, std::vector<int>(p));
  for(unsigned int i=0; i<X.size(); i++) {
    XtMat[i] = sample(X[i]);
  }
  return(XtMat);
}

#endif
