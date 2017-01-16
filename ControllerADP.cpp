#include <iostream>
#include <math.h>
#include "ControllerADP.h"
#include "Controllers.h"
#include "Matrix.h"
#include "SquareMatrix.h"
#include "MatrixCalc.h"
#include "AlgorithmADP.h"
#include "AlgorithmVI.h"
#include "AlgorithmPI.h"

namespace ADP{


	ControllerADP::ControllerADP(const unsigned int n, const unsigned int m, const SquareMatrix& Q, const SquareMatrix& R, const SymmetricMatrix& P, Step* stepf, const double delta, const Matrix& K)
		:mK0(K), mKadp(n,m), mQ(Q), mR(R), mP(P), mdelta(delta), itx(mxx.begin()), itxx(mIxx.begin()), itxu(mIxu.begin()),
		mThetaInv(m*n+n*(n+1)/2,m*n+n*(n+1)/2), mBigV(m*n+n*(n+1)/2,0),mBigTheta(new Matrix(m*n+n*(n+1)/2,m*n+n*(n+1)/2)),mBigr(new Matrix(n*n,m*n+n*(n+1)/2)),
		mADPalg(new AlgorithmVI(mQ,mR,P,stepf))
	{
		long double eps = 1e-10;
		mThetaInv=mThetaInv+1/eps; 
		//disp(mBigV);
		//mADPalg.reset(new AlgorithmPI);

	}


	ControllerADP::ControllerADP(const unsigned int n, const unsigned int m, const SquareMatrix& Q, const SquareMatrix& R, const SymmetricMatrix& P, Step* stepf, const double delta)
		:mK0(n,m), mKadp(n,m), mQ(Q), mR(R), mP(P), mdelta(delta), itx(mxx.begin()), itxx(mIxx.begin()), itxu(mIxu.begin()),
		mThetaInv(m*n+n*(n+1)/2,m*n+n*(n+1)/2), mBigV(m*n+n*(n+1)/2,0),mBigTheta(new Matrix(m*n+n*(n+1)/2,m*n+n*(n+1)/2)),mBigr(new Matrix(n*n,m*n+n*(n+1)/2)),
		mADPalg(new AlgorithmVI(mQ,mR,P,stepf))
	{
		long double eps = 1e-10;
		mThetaInv=mThetaInv+1/eps; 
		//mADPalg.reset(new AlgorithmPI);

	}

	ControllerADP::ControllerADP(const unsigned int n, const unsigned int m, const SquareMatrix& Q, const SquareMatrix& R, const double delta, const Matrix& K)
	:mK0(K), mKadp(K), mQ(Q), mR(R), mP(Q*0), mdelta(delta), itx(mxx.begin()), itxx(mIxx.begin()), itxu(mIxu.begin()),
		mThetaInv(m*n+n*(n+1)/2,m*n+n*(n+1)/2), mBigV(m*n+n*(n+1)/2,0),mBigTheta(new Matrix(m*n+n*(n+1)/2,m*n+n*(n+1)/2)),mBigr(new Matrix(n*n,m*n+n*(n+1)/2)),
		mADPalg(new AlgorithmPI(mQ,mR,K))
	{
		long double eps = 1e-10;
		mThetaInv=mThetaInv+1/eps; 
		//mADPalg.reset(new AlgorithmPI);

	}

	const std::vector<double> ControllerADP::input(const std::vector<double>& x, const double dt,  const double t, noise noif)
	{	
		if(mxx.size()==0) 
		{
			mxx.push_back(vec(kProd(x,x)));
			++itx;
		}else {
			mxx.push_back(vec(kProd(x,x)));
		}

		if(mIxx.size()==0) 
		{
			mIxx.push_back(dt*vecs(prod(x,x)));
			//std::cout << "test post" << std::endl;
			++itxx;
		}
		else {
			//std::vector<double> tem(mIxx.back()+dt*vecs(prod(x,x)));
			//mIxx.push_back(tem);
			mIxx.push_back(mIxx.back()+dt*vecs(prod(x,x)));
		}


		//Matrix matOut(mK0 * x);
		//std::vector<double> u(mR.size()[0],sin(t));
		//noise noif=&sinusoidal;
		std::vector<double> u = noif(mR.size()[0],t);
		//std::vector<double> u(vec(mK0 * x));

		//mu.push_back(u);

		if(mIxu.size()==0) 
		{
			mIxu.push_back(2*dt*vec(kProd(x,mR*u)));
			++itxu;
		} else{
			//std::vector<double> tem2=vec(mIxu.back()+2*dt*kProd(x,mR*u));
			//mIxu.push_back(tem2);
			mIxu.push_back(mIxu.back()+2*dt*vec(kProd(x,mR*u)));
		}

		//std::cout << mxx.size() << ',' <<t << std::endl;


		if (t > mdelta && mxx.size()>1 && mIxx.size()>1 && mIxu.size()>1){
			//std::cout << mdelta << std::endl;
			mdelta +=0.1;
			itx = mxx.erase(itx);
			itxx = mIxx.erase(itxx);
			itxu = mIxu.erase(itxu);
			//++itx;
			//++itxx;
			//++itxu;

			std::vector<double> vec0 = mxx.back() - *itx;
			std::vector<double> vec1;
			vec1.reserve(itxx->size()+itxu->size());
			vec1 = mIxx.back() - *itxx;
			std::vector<double> vec2 = mIxu.back() - *itxu;
			//std::vector<double> vec1 = mIxx.back() - *itxx;
			vec1.insert( vec1.end(), vec2.begin(), vec2.end() );
			//mTheta.push_back(vec1);
			//mXi.push_back(vec0);

			itx = mxx.end();
			itxx = mIxx.end();
			itxu = mIxu.end();
			--itx;
			--itxx;
			--itxu;

			//disp(mBigV);
			//mThetaInv.disp();
			std::vector<double> mBigVold(mBigV);
			//*mBigTheta = *mBigTheta + prod(vec1,vec1); // half online case;
			*mBigr= *mBigr + prod(vec1,vec0);// half online case;
			//std::cout<< double(vec0*vec(mP)) << std::endl;
			//disp(vec1);

			LS(vec1, double(vec0*vec(mP)));
			//disp(mBigV);
			//(mThetaInv * *mBigr * vec(mP)).disp(); 
			//(mThetaInv * *mBigr ).disp(); 
			//std::cout << (mBigV-mBigVold)*(mBigV-mBigVold)<<std::endl;

			//if((mBigV-mBigVold)*(mBigV-mBigVold)<1e-10)
			//{
			// online method
			std::vector<Matrix> optResult = mADPalg->onlineI(mBigV);
			mP = optResult[1];
			mKadp= optResult[2];
			mBigV = vec(mThetaInv * *mBigr * vec(mP));
			mP.disp();
			//mBigr->disp();
			//mKadp.disp();
			//
			//
			if((mBigV-mBigVold)*(mBigV-mBigVold)<1e-10)
			{
				long double eps = 1e-10;
				mThetaInv=mThetaInv*0+1/eps; 
				mBigV = mBigV * 0;
				mxx.clear();
				mIxx.clear();
				mIxu.clear();
				itx = mxx.begin();
				itxx = mIxx.begin();
				itxu = mIxu.begin();
				unsigned int n = mQ.size()[0];
				unsigned int m = mR.size()[0];
				mBigr.reset(new Matrix(n*n,m*n+n*(n+1)/2));
				mADPalg->resetStep();
			}


			// half online half offline method
			//for (int k=1;k<=2000;++k){
			//std::vector<Matrix> optResult = mADPalg->onlineI(mBigV, mQ, mR);
			//mP = optResult[1];
			//mKadp= optResult[2];
			//mP.disp();
			//mBigV = vec(mThetaInv * *mBigr * vec(mP));
			//}

			//}
		}

		return u;

	}

	void ControllerADP::dispAll(){

		std::cout <<"what is in mxx" << std::endl;
		for (auto itx = mxx.begin();itx!=mxx.end();++itx)
		{
			disp(*itx);
		}
		std::cout <<"what is in mIxx" << std::endl;
		for (auto itxx = mIxx.begin();itxx!=mIxx.end();++itxx)
		{
			disp(*itxx);
		}
		std::cout <<"what is in mIxu" << std::endl;
		for (auto itxu = mIxu.begin();itxu!=mIxu.end();++itxu)
		{
			disp(*itxu);
		}

	}


	void ControllerADP::LS(const std::vector<double>& phi, const double d)
	{
		mThetaInv = mThetaInv - 1 / (1 + double(t(phi)*mThetaInv*phi)) * mThetaInv * phi * t(mThetaInv * phi);
		double alpha = d - mBigV * phi;
		std::vector<double> g(vec(mThetaInv * phi));
		//*mBigr= *mBigr + phi * d;// half online case;
		mBigV = mBigV + g * alpha;
		//std::cout<< double(mBigV*phi)-d << std::endl;
	}
}