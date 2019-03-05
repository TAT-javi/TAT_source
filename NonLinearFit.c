#define NRANSI
#include "nrutil.h"
#include <stdio.h>
#include <math.h>


void NonlinearFit(float x[], float y[], float z[], float sig[], int xn, int yn, int Raw, float a[], int ia[],
	int ma, float **covar, float **alpha, float *chisq, float maxpoint, float minpoint,
	void (*funcs)(float, float, float [], float *, float [], int), float *alamda)
{
	void covsrt(float **covar, int ma, int ia[], int mfit);
	void gaussj(float **a, int n, float **b, int m);
	void mrqcof(float x[], float y[], float z[], float sig[], int xn, int yn, int Raw, float a[],
		int ia[], int ma, float **alpha, float beta[], float *chisq, float maxpoint, float minpoint,
		void (*funcs)(float, float, float [], float *, float [], int));
	int j,k,l;
	static int mfit;
	static float ochisq,*atry,*beta,*da,**oneda;

	if (*alamda < 0.0) 
	{
		atry=vector(1,ma);
		beta=vector(1,ma);
		da=vector(1,ma);
		
		for (mfit=0,j=1;j<=ma;j++){if (ia[j]) mfit++;}
		
		oneda=matrix(1,mfit,1,1);
		
		*alamda=0.001;
		
		mrqcof(x,y,z,sig,xn,yn,Raw,a,ia,ma,alpha,beta,chisq,maxpoint,minpoint,funcs);
		ochisq=(*chisq);
		
		for (j=1;j<=ma;j++) atry[j]=a[j];
	}
	for (j=1;j<=mfit;j++) 
	{
		for (k=1;k<=mfit;k++) 
			covar[j][k]=alpha[j][k];
		
		covar[j][j]=alpha[j][j]*(1.0+(*alamda));
		oneda[j][1]=beta[j];
	}
	
	gaussj(covar,mfit,oneda,1);
	for (j=1;j<=mfit;j++) 
		da[j]=oneda[j][1];
	
	if (*alamda == 0.0) 
	{
		covsrt(covar,ma,ia,mfit);
		covsrt(alpha,ma,ia,mfit);
		free_matrix(oneda,1,mfit,1,1);
		free_vector(da,1,ma);
		free_vector(beta,1,ma);
		free_vector(atry,1,ma);
		return;
	}
	for (j=0,l=1;l<=ma;l++) 
	{
		if (ia[l]) 
		{
			atry[l]=a[l]+da[++j];
		}
	}
	
	
	mrqcof(x,y,z,sig,xn,yn,Raw,atry,ia,ma,covar,da,chisq,maxpoint,minpoint,funcs);
	
	if (*chisq < ochisq) 
	{
		*alamda *= 0.1;
		ochisq=(*chisq);
		for (j=1;j<=mfit;j++) 
		{
			for (k=1;k<=mfit;k++) 
				alpha[j][k]=covar[j][k];
			
			beta[j]=da[j];
		}
		for (l=1;l<=ma;l++) 
			a[l]=atry[l];
	} 
	else 
	{
		*alamda *= 10.0;
		*chisq=ochisq;
	}
}




void mrqcof(float x[], float y[], float z[], float sig[], int xn, int yn, int Raw,float a[], int ia[],
	int ma, float **alpha, float beta[], float *chisq, float maxpoint, float minpoint,
	void (*funcs)(float, float, float [], float *, float [], int))
{
	int i,j,k,l,m,n,mfit=0,itemp;
	float zmod,wt,sig2i,dz,*dzda;

	dzda=vector(1,ma);
	for (j=1;j<=ma;j++){if (ia[j]) mfit++;}
	
	for (j=1;j<=mfit;j++) 
	{
		for (k=1;k<=j;k++) 
			alpha[j][k]=0.0;
			
		beta[j]=0.0;
	}
	*chisq=0.0;
// 	for (i=0;i<xn;i++) 
	for (i=1;i<=xn;i++)
	{
// 		for(n=0;n<yn;n++)
		for(n=1;n<=yn;n++)
		{
			(*funcs)(x[i],y[n],a,&zmod,dzda,ma);
			 //zmod = GaussFit( x[i], y[n], a, dzda);
			//		sig2i=1.0/(sig[i]*sig[i]);
			itemp=x[i]+y[n]*Raw;
 			//printf("x=%.1f, y=%.1f, total_coord=%d, z=%.1lf ==> fit=%f\n",x[i],y[n],itemp,z[itemp],zmod);
			dz=z[itemp]-zmod;
// 			printf("dz = %.2f\n",dz);
			if (z[itemp]>maxpoint) sig2i=0;
			else if (z[itemp]<minpoint) sig2i=0;
			else sig2i=1;
			for (j=0,l=1;l<=ma;l++) 
			{
				if (ia[l]) 
				{
					wt=dzda[l]*sig2i;
					for (j++,k=0,m=1;m<=l;m++)
					{
						if (ia[m]) alpha[j][++k] += wt*dzda[m];
					}
					beta[j] += dz*wt;
				}
			}
			*chisq += dz*dz*sig2i;
		}
	}
	for (j=2;j<=mfit;j++)
	{
		for (k=1;k<j;k++) 
			alpha[k][j]=alpha[j][k];
	}
	free_vector(dzda,1,ma);
}

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

void gaussj(float **a, int n, float **b, int m)
{
	int *indxc,*indxr,*ipiv;
	int i,icol,irow,j,k,l,ll;
	float big,dum,pivinv,temp;

	indxc=ivector(1,n);
	indxr=ivector(1,n);
	ipiv=ivector(1,n);
	for (j=1;j<=n;j++) ipiv[j]=0;
	for (i=1;i<=n;i++) {
		big=0.0;
		for (j=1;j<=n;j++)
			if (ipiv[j] != 1)
				for (k=1;k<=n;k++) {
					if (ipiv[k] == 0) {
						if (fabs(a[j][k]) >= big) {
							big=fabs(a[j][k]);
							irow=j;
							icol=k;
						}
					}
				}
		++(ipiv[icol]);
		if (irow != icol) {
			for (l=1;l<=n;l++) SWAP(a[irow][l],a[icol][l])
			for (l=1;l<=m;l++) SWAP(b[irow][l],b[icol][l])
		}
		indxr[i]=irow;
		indxc[i]=icol;
		if (a[icol][icol] == 0.0) nrerror("gaussj: Singular Matrix");
		pivinv=1.0/a[icol][icol];
		a[icol][icol]=1.0;
		for (l=1;l<=n;l++) a[icol][l] *= pivinv;
		for (l=1;l<=m;l++) b[icol][l] *= pivinv;
		for (ll=1;ll<=n;ll++)
			if (ll != icol) {
				dum=a[ll][icol];
				a[ll][icol]=0.0;
				for (l=1;l<=n;l++) a[ll][l] -= a[icol][l]*dum;
				for (l=1;l<=m;l++) b[ll][l] -= b[icol][l]*dum;
			}
	}
	for (l=n;l>=1;l--) {
		if (indxr[l] != indxc[l])
			for (k=1;k<=n;k++)
				SWAP(a[k][indxr[l]],a[k][indxc[l]]);
	}
	free_ivector(ipiv,1,n);
	free_ivector(indxr,1,n);
	free_ivector(indxc,1,n);
}

void covsrt(float **covar, int ma, int ia[], int mfit)
{
	int i,j,k;
	float swap,temp;

	for (i=mfit+1;i<=ma;i++)
		for (j=1;j<=i;j++) covar[i][j]=covar[j][i]=0.0;
	k=mfit;
	for (j=ma;j>=1;j--) {
		if (ia[j]) {
			for (i=1;i<=ma;i++) SWAP(covar[i][k],covar[i][j])
			for (i=1;i<=ma;i++) SWAP(covar[k][i],covar[j][i])
			k--;
		}
	}
}

#undef SWAP
void GaussFit(float x, float y, float a[], float *z, float dzda[], int ma)
{
	float argx, argy, ex, cross;
	int i=0;
	
	*z=0.0;
	for (i=0;i<ma;i+=6) 
	{
		argx=(x-a[i+2])/a[i+4];
		argy=(y-a[i+3])/a[i+5];
		cross=a[i+6]*(x-a[i+2])*(y-a[i+3]);
		ex=exp(-argx*argx-argy*argy-cross);
		*z += a[i+1]*ex;
		dzda[i+1]=ex;
		dzda[i+2]=a[i+1]*ex*(2*argx/a[i+4]+a[i+6]*(y-a[i+3]));
		dzda[i+3]=a[i+1]*ex*(2*argy/a[i+5]+a[i+6]*(x-a[i+2]));
		dzda[i+4]=a[i+1]*ex*2*argx*argx/a[i+4];
		dzda[i+5]=a[i+1]*ex*2*argy*argy/a[i+5];
		dzda[i+6]=-a[i+1]*ex*(x-a[i+2])*(y-a[i+3]);
	}
}

#undef NRANSI


