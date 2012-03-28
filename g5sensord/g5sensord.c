/*+FHDR-----------------------------------------------------------------------
* Copyright (c) 2011, 2012 Freescale Semiconductor, Inc.
* Freescale Semiconductor Confidential Proprietary
* ----------------------------------------------------------------------------
* FILE NAME : main.c
* DEPARTMENT : SENSORS
* AUTHOR : MARK PEDLEY
* ----------------------------------------------------------------------------
* REVIEW(S) :                                        
* ----------------------------------------------------------------------------
* VERSION DATE AUTHOR DESCRIPTION
* V1.0: 20 FEB 2012: MARK PEDLEY
* ----------------------------------------------------------------------------
* KEYWORDS: MAGNETIC CALIBRATION
* ----------------------------------------------------------------------------
* PURPOSE:
* Tilt-compensated e-compass with 7, 4 element calibration.
* Stand-alone file using sensor simulation to verify algorithms.
* ----------------------------------------------------------------------------
* COMMENTS:
* This code is a vanilla C console application.
* It is intended that customers:
* 1. Compile and run this on the destination embedded system using console I/O
* to verify that the eCompass and calibration algorithms execute correctly
* using simulation of the magnetometer and accelerometer sensors and then
* 2. Replace the sensor simulation function with real I2C drivers for the
* physical magnetometer and accelerometer sensors.
*
* IMPORTANT: The most likely source of error moving from simulated to physical
* sensors is the alignment of the sensor coordinate systems. See the user
* manual  for more details.
* -FHDR---------------------------------------------------------------------*/

//#include <stdafx.h>
#include <stdio.h> 
//#include <conio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h> 
#include <time.h>  
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/i2c/bma220.h>
#include <linux/i2c/mmc31xx.h>
#include <linux/i2c/mecs.h>


/* maximum calibration model size supported in this code */
#define MAXCALELEMENTS 7

/* mixing coefficient to permit vertical smartphone operation */
//#define FMU 0.00F						/* value of 0.0F used to verify correct operation in this code */
#define FMU 0.04F						/* stabilizes feCompass at high pitch with reduced accuracy */ 

/* constants to minimise numerical noise in the SVD algorithm */
#define fSVDscaling 0.02F				/* approx normalises geomagnetic field 50uT */
#define finvSVDscaling 50.0F			/* inverse of fSVDscaling */

/* fixed constants for array dimensioning */ 
#define DegToRad 0.0174532925199433F	/* deg to rad conversion */
#define RadToDeg 57.2957795130823F		/* rad to deg conversion */
#define ABSMAXPITCH 3					/* maximum pitch dimension in smart FIFO / constellation */
#define ABSMAXROLL 6					/* maximum roll dimension in smart FIFO / constellation */
#define ABSMAXYAW 6						/* maximum yaw dimension in smart FIFO / constellation */
#define ABSMAXPITCHROLLYAW 108			/* must be set to product of dimensions above */
#define TRUE 1							/* logical true */
#define FALSE 0							/* logical false */
#define ABSMAXEQUATIONS 64				/* ensure MAXEQUATIONS <= ABSMAXEQUATIONS */

/* run time coefficients which could be set from a user interface but hard-wired here */
int CURRENTMAXPITCH = 3;				/* current pitch dimension in smart FIFO / constellation */
int CURRENTMAXROLL = 6;					/* current roll dimension in smart FIFO / constellation */
int CURRENTMAXYAW = 6;					/* current yaw dimension in smart FIFO / constellation */
int CALUPDACTIVE = TRUE;				/* value TRUE (normal use) to enable updating of calibration */
int MINEQUATIONS = 16;					/* minimum number of equations for calibration to run */
int MAXEQUATIONS = 64;					/* maximum number of equations used for calibration */                                                           
int CALINTERVAL = 40;					/* interval in samples between re-computation of the calibration */
int HARDCORRACTIVE = TRUE;				/* flag to enable subtraction of hard iron interference estimate */
int SOFTCORRACTIVE = TRUE;				/* flag to enable removal of soft iron interference estimate */
int TILTCORRACTIVE = TRUE;				/* flag for applying tilt correction */
float ANGLE_LPF_FPU = 0.0625;			/* recip of angle LPF impulse response (16 samples here) */

/* hardware abstraction layer parameters used to align sensor x, y, z axes together */
/* accelerometer HAL coefficients set to default identity matrix */
int ACCHAL00 = 1;			
int ACCHAL01 = 0;
int ACCHAL02 = 0;
int ACCHAL10 = 0;
int ACCHAL11 = 1;
int ACCHAL12 = 0;
int ACCHAL20 = 0;
int ACCHAL21 = 0;
int ACCHAL22 = -1;

/* magnetometer HAL coefficients set to default identity matrix */
int MAGHAL00 = 1;					
int MAGHAL01 = 0;
int MAGHAL02 = 0;
int MAGHAL10 = 0;
int MAGHAL11 = 1;
int MAGHAL12 = 0;
int MAGHAL20 = 0;
int MAGHAL21 = 0;
int MAGHAL22 = 1; 

/* global scalars */
float fPsi, fThe, fPhi;				/* raw yaw, pitch, roll angles in deg */
float fLPPsi, fLPThe, fLPPhi;		/* low pass filtered yaw, pitch, roll angles in deg */
float fdelta;						/* raw calculated geomagnetic inclination angle */
float fLPdelta;						/* low pass filtered calculated inclination angle */
float fBpx, fBpy, fBpz;				/* raw magnetometer data in uT */
float fBcx, fBcy, fBcz;				/* mag data in uT after calibration correction */
float fBfx, fBfy, fBfz;				/* mag data in uT after tilt correction */
float fGpx, fGpy, fGpz;				/* raw accel data in g */
int SOLUTIONSIZE;					/* calibration model size: 4, 7 elements */
float fVx, fVy, fVz;				/* computed hard iron calibration in uT */
float fB;                           /* fitted geomagnetic field magnitude in uT */
float det;                          /* matrix determinant */
float fFitErrorpc;                  /* fit error as a percentage */
int validcal;						/* flag to denote at least one cal has been performed */
int loopcounter;					/* global counter incrementing each iteration of compass */
int ConstCount;                     /* number of magnetometer readings in  the constellation */
int earliestsample;                 /* earliest sample number used in current calibration */
int nequations;                     /* number of equations in SVD */
float SimVx, SimVy, SimVz;			/* simulation model hard iron */
float SimB;							/* geomagnetic field strength */
float Simdelta;						/* geomagnetic field inclination (deg) */

/* global arrays */ 
float xfinvW[3][3], *finvW[3];										/* computed inverse soft iron matrix size */
float xinvSimW[3][3], *invSimW[3];									/* sensor simulation inverse soft iron matrix */
float xSimW[3][3], *SimW[3];										/* sensor simulation forward soft iron matrix */
float xfX[ABSMAXEQUATIONS][MAXCALELEMENTS], *fX[ABSMAXEQUATIONS];	/* matrix of measurements X */
float xfY[ABSMAXEQUATIONS][1], *fY[ABSMAXEQUATIONS];				/* 4 element model dependent variables */
float xU[ABSMAXEQUATIONS][MAXCALELEMENTS], *U[ABSMAXEQUATIONS];		/* SVD: X = U S GkY^T */
float xS[MAXCALELEMENTS][1], *S[MAXCALELEMENTS];					/* SVD: X = U S GkY^T */
float xGkY[MAXCALELEMENTS][MAXCALELEMENTS], *GkY[MAXCALELEMENTS];	/* SVD: X = U S GkY^T */
float xA[3][3], *A[3];												/* ellipsoid matrix A */
float xinvA[3][3], *invA[3];										/* inverse of ellipsoid matrix A */
float xfBeta4[4][1], *fBeta4[4];									/* 4 element model solution vector */
float xftmpA3x3[3][3], *ftmpA3x3[3];								/* scratch 3x3 matrix */
float xftmpA4x1[4][1], *ftmpA4x1[4];								/* scratch 4x1 matrix */
float xftmpA4x4[4][4], *ftmpA4x4[4];								/* scratch 4x4 matrix */
float xftmpB4x4[4][4], *ftmpB4x4[4];								/* scratch 4x4 matrix */
float xftmpAMAXx1[ABSMAXEQUATIONS][1], *ftmpAMAXx1[ABSMAXEQUATIONS];	/* scratch MAXx1 matrix */
float fConBpx[ABSMAXPITCH][ABSMAXROLL][ABSMAXYAW];					/* float smart FIFO */
float fConBpy[ABSMAXPITCH][ABSMAXROLL][ABSMAXYAW];					/* float smart FIFO */
float fConBpz[ABSMAXPITCH][ABSMAXROLL][ABSMAXYAW];					/* float smart FIFO */
int ConIndex[ABSMAXPITCH][ABSMAXROLL][ABSMAXYAW];					/* sample index to time entry */
int ScratchConIndex[ABSMAXPITCHROLLYAW];							/* scratch array for time sorting */

/* function prototypes */
void fSixDOFSensorDrivers(int k);
void feCompass(float fBx, float fBy, float fBz, float fGx, float fGy, float fGz);
void fModuloLPF(float Angle, float *pLPFAngle);
void fUpdateConstellation(void);
void fUpdateCalibration7SVD(void);
void fUpdateCalibration4INV(void); 
void heapSort(int numbers[], int array_size);
void siftDown(int numbers[], int root, int bottom);
void ResetCalibrationFunc(void);
void fInvertHardandSoftIron(void);
void fmatrixAeqBxC(float **A, float **B, float **C, int rB, int cBrC, int cC);
void fmatrixAeqTrBxB(float **A, float **B, int r, int c);
void fmatrixAeqTrBxC(float **A,  float **B,  float **C, int rBrC, int cB, int cC);
void fmatrixAeqI(float **A, int rc);
void fmatrixPrintA(float **A, int r1, int r2, int c1, int c2);
float f3x3matrixDetA(float **inp);
void f3x3matrixAeqInvB(float **A, float **B);
void f4x4matrixAeqInvB(float **A, float **B);
void fmatrixAeqAxScalar(float **A, float Scalar, int r, int c);
void eigensort(float **eigval, float **eigvec, int n);
float pythag(float a, float b);
void SVDcompute(float **mat, int m, int n, float **w, float **v);

int fd_bma, fd_mmc;
int aflag, mflag, oflag, prev_aflag, prev_mflag, prev_oflag=0;
int poll_delay=50;

int main(int argc, char *argv[])
{
	/* local variables */
	int fd_ecompass;					/* file handle */
	int command;						/* keyboard command selected */
	int i;							/* loop counter */
	int niterations;					/* number of eCompass iterations to be performed */
	float ftmpx, ftmpy, ftmpz;				/* scratch variables */
	int ypr[13]={0};					/* sensor events */
	int tmpyaw, tmppitch, tmproll=0;			/* temporary orientation value */
	FILE *fp;

	/* apply the tweak for C's limitation on functions receiving variable size arrays */
	/* slight overhead of additional storage since 2D arrays are replaced with 1D arrays */
	/* of pointers to 1D arrays */
	/* 3 row arrays */
	for (i = 0; i < 3; i++)
	{
		invSimW[i] = xinvSimW[i]; 	
		SimW[i] = xSimW[i];
		finvW[i] = xfinvW[i];
		A[i] = xA[i];
		invA[i] = xinvA[i];
		ftmpA3x3[i] = xftmpA3x3[i];
	}
	/* 4 row arrays */
	for (i = 0; i < 4; i++)
	{
		fBeta4[i] = xfBeta4[i];
		ftmpA4x1[i] = xftmpA4x1[i];
		ftmpA4x4[i] = xftmpA4x4[i];
		ftmpB4x4[i] = xftmpB4x4[i];
	}
	/* MAXCALELEMENTS row arrays */
	for (i = 0; i < MAXCALELEMENTS; i++)
	{
		S[i] = xS[i];
		GkY[i] = xGkY[i];
	}
	/* MAXEQUATIONS row arrays */
	for (i = 0; i < MAXEQUATIONS; i++)
	{
		fX[i] = xfX[i];
		U[i] = xU[i];
		fY[i] = xfY[i];
		ftmpAMAXx1[i] = xftmpAMAXx1[i];
	} 

	/* for safety, reset sensor simulation hard iron to zero and soft iron to identity matrix */ 
	/* in case the simulation is executed without entering these values */
	fmatrixAeqI(invSimW, 3);
	fmatrixAeqI(SimW, 3);
	SimVx = SimVy = SimVz = 0.0F;

	/* for safety, set the geomagnetic field to safe value 50uT and 50 deg */
	/* in case the simulation is executed without entering these values */
	SimB = 50.0F;
	Simdelta = 50.0F;

	/* reset computed calibration and other structures */ 
	ResetCalibrationFunc(); 

	/* seed the random number generator */
	srand((unsigned int) time(NULL));

	/* control loop terminating with option 99 */
	printf("\ng5sensord - Freescale eCompass and Magnetic Calibration Software (OuNao mod)\n");

	// OuNao inclusions - start
	ResetCalibrationFunc();
	SOLUTIONSIZE=7;
	
	// save/get calibration values
	if(( fp = fopen("/data/misc/sensors/g5s_calib", "rb+")) == NULL) {
	  printf("Cannot open file. Creating new file...\n");
	  fp = fopen("/data/misc/sensors/g5s_calib", "wb+");
	}
	if( fp != NULL) {
	  fread(&fVx, sizeof(float), 1, fp);
	  fread(&fVy, sizeof(float), 1, fp);
	  fread(&fVz, sizeof(float), 1, fp);
	}
	fclose(fp);
	//printf("\nfVx= %f, fVy= %f, fVz= %f", fVx, fVy, fVz);
	
	/* keyboard command interpreter */
	for (;;)
	{
	fd_ecompass = open("/dev/ecompass_ctrl", O_RDWR);
	if (ioctl(fd_ecompass, ECOMPASS_IOC_GET_AFLAG, &aflag)<0)
	{
	  printf("eCompass ioctl error\n");
	};
	if (ioctl(fd_ecompass, ECOMPASS_IOC_GET_MFLAG, &mflag)<0)
	{
	  printf("eCompass ioctl error\n");
	};
	if (ioctl(fd_ecompass, ECOMPASS_IOC_GET_OFLAG, &oflag)<0)
	{
	  printf("eCompass ioctl error\n");
	};
	
	/* call sensor driver simulation to get float acc fGpxyz (g) and mag fBpxyz (uT) data */
	//printf("\nIteration: %6d",  i);
	fSixDOFSensorDrivers(i);

	/* NED magnetometer HAL to correct for package orientation on PCB and gain */
	ftmpx = MAGHAL00 * fBpx + MAGHAL01 * fBpy + MAGHAL02 * fBpz;
	ftmpy = MAGHAL10 * fBpx + MAGHAL11 * fBpy + MAGHAL12 * fBpz;
	ftmpz = MAGHAL20 * fBpx + MAGHAL21 * fBpy + MAGHAL22 * fBpz;
	fBpx = ftmpx;
	fBpy = ftmpy;
	fBpz = ftmpz;

	/* NED accelerometer HAL to correct for package orientation on PCB and gain */
	ftmpx = ACCHAL00 * fGpx + ACCHAL01 * fGpy + ACCHAL02 * fGpz;
	ftmpy = ACCHAL10 * fGpx + ACCHAL11 * fGpy + ACCHAL12 * fGpz;
	ftmpz = ACCHAL20 * fGpx + ACCHAL21 * fGpy + ACCHAL22 * fGpz;            
	fGpx = ftmpx;
	fGpy = ftmpy;
	fGpz = ftmpz;

	/* remove hard and soft iron terms from Bp (uT) to get calibrated data Bc (uT) */
	fInvertHardandSoftIron();
	
	if (oflag==1)
	{
	/* pass the accel and calibrated mag data to the eCompass */
	feCompass(fBcx, fBcy, fBcz, fGpx, fGpy, fGpz);
	//printf("\nf6DOFOutp: Phi %6.2f The %6.2f Psi %6.2f delta %6.2f", fPhi, fThe, fPsi, fdelta);
	//printf("\nf6DOFOutp: LPPhi %6.2f LPThe %6.2f LPPsi %6.2f LPdelta %6.2f", fLPPhi, fLPThe, fLPPsi, fLPdelta);

	/* update the constellation */
	fUpdateConstellation();

	/* update the calibration if update is enabled */
	if (CALUPDACTIVE)
	{
		/* check for enough data in constellation for a calibration */
		if (ConstCount >= MINEQUATIONS)
		{
			/*  calibrate if this will be the first calibration or every CALINTERVAL iterations */
			if ((!validcal) || (validcal && !(loopcounter % CALINTERVAL)))
			{
				/* 7 point calibration */
				if (SOLUTIONSIZE == 7)
				{
					fUpdateCalibration7SVD();
				}
				/* 4 point calibration */
				else if (SOLUTIONSIZE == 4)  
				{
					fUpdateCalibration4INV();
				}
				/* defensive default to 4 point calibration */
				else
				{
					fUpdateCalibration4INV();
				}
			}
		}
		else /* still too few entries in constellation for calibration */
		{
			//printf("\n%d entries in constellation is too few for calibration", ConstCount);
		}
	} /* end of test for active calibration flag */
	
	  //printf("\nfBpx= %f, fBcx= %f, fBpy= %f, fBcy= %f, fBpz= %f, fBcz= %f, ", fBpx, fBcx, fBpy, fBcy, fBpz, fBcz);
	}
	//tmpyaw=(int)fPsi;
	//tmproll=(int)fThe;
	//tmppitch=(int)fPhi;
	//if (tmpyaw<0) tmpyaw+=360;
	if (fPsi<0) fPsi+=360;
	ypr[0]=fGpy*-32768;
	ypr[1]=fGpx*-32768;
	ypr[2]=fGpz*32768;
	ypr[4]=(fBcy*32768)/100;
	ypr[5]=(fBcx*32768)/100;
	ypr[6]=(fBcz*32768)/-100;
	ypr[8]=(fPsi*65536)/360;
	ypr[9]=(fThe*65536)/-360;
	ypr[10]=(fPhi*65536)/-360;
	ioctl(fd_ecompass, ECOMPASS_IOC_SET_YPR, &ypr);
	//printf("\nypr= %i, %i, %i, %i, %i, %i, %i, %i, %i", ypr[0], ypr[1], ypr[2], ypr[4], ypr[5], ypr[6], ypr[8], ypr[9], ypr[10]);
	close(fd_ecompass);  
	
	if ( argc == 2 ) /* argc should be 2 for correct execution */
	{
	  poll_delay=atoi(argv[1]);
	}
	usleep(poll_delay*1000);
	}
	return (1);
}

/* (FLOAT) map the uncalibrated data Bp (uT) onto calibrated data Bc (uT) */
void fInvertHardandSoftIron()
{
	/* local  variables */
	float ftmpx, ftmpy, ftmpz;

	/* remove the computed hard iron offset if enabled */
	if (HARDCORRACTIVE)
	{
		fBcx = fBpx - fVx;
		fBcy = fBpy - fVy;
		fBcz = fBpz - fVz;
	}
	else
	{
		fBcx = fBpx;
		fBcy = fBpy;
		fBcz = fBpz;
	}

	/* remove the computed soft iron offset if enabled */
	if (SOFTCORRACTIVE)
	{
		ftmpx = finvW[0][0] * fBcx + finvW[0][1] * fBcy + finvW[0][2] * fBcz;
		ftmpy = finvW[1][0] * fBcx + finvW[1][1] * fBcy + finvW[1][2] * fBcz;
		ftmpz = finvW[2][0] * fBcx + finvW[2][1] * fBcy + finvW[2][2] * fBcz;
		fBcx = ftmpx; 
		fBcy = ftmpy;	
		fBcz = ftmpz;	
	}
	return;
}

/* (FLOAT) tilt-compensated e-Compass function */
void feCompass(float fBx, float fBy, float fBz, float fGx, float fGy, float fGz)
{
	/* stack variables */
	/* fBx, fBy, fBz: float magnetometer sensor in any units */
	/* fGx, fGy, fGz: float accelerometer sensor in any units */

	/* local variables */
	float sinAngle, cosAngle;			/* sine and cosine */

	/* calculate roll angle Phi (-180deg, 180deg) */
	fPhi = atan2(fGy, ((fGz >= 0.0F) ? 1.0F : -1.0F) * sqrt(fGz * fGz + FMU * fGx * fGx)) * RadToDeg;
	/* calculate sin(Phi) and cos(Phi) */
	sinAngle = sin(fPhi * DegToRad);   /* sin(Phi) */
	cosAngle = cos(fPhi * DegToRad);   /* cos(Phi) */
	/* de-rotate by roll angle Phi */
	fBfy = fBy * cosAngle - fBz * sinAngle;
	fBz = fBy * sinAngle + fBz * cosAngle;
	fGz = fGy * sinAngle + fGz * cosAngle;

	/* check for division by zero and calculate pitch angle Theta (-90deg, 90deg) */
	if (fGz == 0.0F) fGz = 1E-10F;
	fThe = atan(-fGx / fGz) * RadToDeg;
	/* calculate sin(Theta) and cos(Theta) */
	sinAngle = sin(fThe * DegToRad);	/* sin(The) */
	cosAngle = cos(fThe * DegToRad);	/* cos(The) */
	/* de-rotate by pitch angle Theta */
	fBfx = fBx * cosAngle + fBz * sinAngle;
	fBfz = -fBx * sinAngle + fBz * cosAngle;

	/* calculate yaw = ecompass angle psi (-180deg, 180deg) with or without tilt compensation */
	if (TILTCORRACTIVE)
		fPsi = atan2(-fBfy, fBfx) * RadToDeg; 
	else
		fPsi = atan2(-fBy, fBx) * RadToDeg; 
	
	/* calculate the geomagnetic inclination angle delta */
	fdelta = atan2(fBfz, sqrt(fBfx * fBfx + fBfy * fBfy)) * RadToDeg;

	/* check for first iteration to initialise low pass filters */
	if (loopcounter == 0)
	{
		fLPPhi = fPhi;
		fLPThe = fThe;
		fLPPsi = fPsi;
		fLPdelta = fdelta;
	}

	/* apply the low pass filter on the roll angle (result will be bounded -180 to 180 deg) */
	fModuloLPF(fPhi, &fLPPhi);

	/* apply the low pass filter on the pitch angle and ensure result is bounded -90 to 90 deg */
	fModuloLPF(fThe, &fLPThe);
	if (fLPThe > 90.0F)
		fLPThe = 180.0F - fLPThe;
	if (fLPThe < -90.0F)
		fLPThe = -180.0F - fLPThe;

	/* apply the low pass filter on the compass / yaw angle (result will be bounded -180 to 180 deg) */
	fModuloLPF(fPsi, &fLPPsi);

	/* apply the low pass filter on the magnetic inclination angle */
	fModuloLPF(fdelta, &fLPdelta);

	return;
}

/* (FLOAT) modulo 360 degree exponential low pass filter for angles */
void fModuloLPF(float fAngle, float *pfLPFAngle)
{
	/* local variables */
	float ftmpAngle;					/* temporary angle */

	/* calculate the modulo 360 deg difference between current and low pass filtered sample */
	ftmpAngle = fAngle - *pfLPFAngle;
	if (ftmpAngle > 180.0F)
		ftmpAngle -= 360.0F;
	if (ftmpAngle < -180.0F)
		ftmpAngle += 360.0F;

	/* calculate the new low pass filtered angle */
	*pfLPFAngle += ANGLE_LPF_FPU * ftmpAngle;

	/* check that the angle remains in range -180 to 180 deg */
	if (*pfLPFAngle > 180.0F)
		*pfLPFAngle -= 360.0F;
	if (*pfLPFAngle < -180.0F)
		*pfLPFAngle += 360.0F;

	return;
}

/* (FLOAT) update the constellation / smart FIFO with most recent data */
void fUpdateConstellation(void)
{
	/* local variables */
	int i;                  /* general loop counter */
	int j, k, l;            /* constellation indices */

	/* find constellation indices */
	j = (int) ((fThe + 90.) * CURRENTMAXPITCH / 180.); 	/* pitch theta range -90. to 90. deg */
	k = (int) ((fPhi + 180.) * CURRENTMAXROLL / 360.); 	/* roll phi range -180. to 180. deg */
	l = (int) ((fPsi + 180.) * CURRENTMAXYAW / 360.);	/* yaw psi range -180. to 180. deg */

	/* bounds safety check in case exactly on boundary */
	if (j >= CURRENTMAXPITCH) j = CURRENTMAXPITCH - 1;		
	if (k >= CURRENTMAXROLL) k = CURRENTMAXROLL - 1;	
	if (l >= CURRENTMAXYAW) l = CURRENTMAXYAW - 1;		

	/* store the magnetometer reading and loop counter into the constellation */
	fConBpx[j][k][l] = fBpx;
	fConBpy[j][k][l] = fBpy;
	fConBpz[j][k][l] = fBpz;
	/* loop counter has valid value 0 at start of first pass */
	ConIndex[j][k][l] = loopcounter++;

	/* update the constellation count */
	i = 0;
	for (j = 0; j < CURRENTMAXPITCH; j++)
		for (k = 0; k < CURRENTMAXROLL; k++)
			for (l = 0; l < CURRENTMAXYAW; l++)                       
				if (ConIndex[j][k][l] != -1)
				{
					i++;
				}
				ConstCount = i;

				return;
}

/* (FLOAT) 7 element calibration using SVD */
void fUpdateCalibration7SVD(void)
{
	int i, j, k, l;					       /* loop counters */
	float fOffsetx, fOffsety, fOffsetz;    /* offset to remove large DC hard iron bias in matrix */
	float ftmpBpx, ftmpBpy, ftmpBpz;       /* scratch variables */
	FILE *fp;

	//printf("\n\nCalculating 7 element SVD calibration at iteration %d with %d in Smart FIFO", loopcounter, ConstCount);

	/* the offsets are guaranteed to be set from the first element but to avoid compiler error */
	fOffsetx = fOffsety = fOffsetz = 0.0F;

	/* should never occur but return with no calibration if too few entries in the constellation */
	if (ConstCount < MINEQUATIONS)
	{
		return;
	}

	/* place the constellation sample number index into a temporary scratch array */
	i = 0;
	for (j = 0; j < CURRENTMAXPITCH; j++)
	{
		for (k = 0; k < CURRENTMAXROLL; k++)
		{
			for (l = 0; l < CURRENTMAXYAW; l++)
			{
				ScratchConIndex[i] = ConIndex[j][k][l];
				i++;
			}
		}
	}
	/* sort the scratch array into ascending order in place */
	heapSort(ScratchConIndex, (int)(CURRENTMAXPITCH * CURRENTMAXROLL * CURRENTMAXYAW));

	/* determine the earliest sample number that is needed to get MAXEQUATIONS: note this may be -1 */     
	/* defensive programming test */
	if (MAXEQUATIONS <= CURRENTMAXPITCH * CURRENTMAXROLL * CURRENTMAXYAW)
	{
		earliestsample = ScratchConIndex[CURRENTMAXPITCH * CURRENTMAXROLL * CURRENTMAXYAW - MAXEQUATIONS];
	}
	else
	{
		earliestsample = 0;
	}

	/* place from MINEQUATIONS to MAXEQUATIONS entries into the measurement matrix */
	i = 0;
	for (j = 0; j < CURRENTMAXPITCH; j++)
	{
		for (k = 0; k < CURRENTMAXROLL; k++)
		{
			for (l = 0; l < CURRENTMAXYAW; l++)
			{
				if ((ConIndex[j][k][l] != -1) && (ConIndex[j][k][l] >= earliestsample))
				{
					/* set tmp to valid data from the constellation */
					ftmpBpx = fConBpx[j][k][l];
					ftmpBpy = fConBpy[j][k][l];
					ftmpBpz = fConBpz[j][k][l];

					/* use first valid constellation entry as estimate (in uT) for offset to help solution */
					if (i == 0)
					{
						fOffsetx = ftmpBpx;
						fOffsety = ftmpBpy;
						fOffsetz = ftmpBpz;
					}

					/* apply the same fixed offset (in bit counts) to all measurement vectors for this iteration */
					ftmpBpx -= fOffsetx;
					ftmpBpy -= fOffsety;
					ftmpBpz -= fOffsetz;

					/* enter into the measurement matrix X scaling to make entries near unity */
					fX[i][0] = ftmpBpx * ftmpBpx * fSVDscaling * fSVDscaling;
					fX[i][1] = ftmpBpy * ftmpBpy * fSVDscaling * fSVDscaling;
					fX[i][2] = ftmpBpz * ftmpBpz * fSVDscaling * fSVDscaling;
					fX[i][3] = ftmpBpx * fSVDscaling;
					fX[i][4] = ftmpBpy * fSVDscaling;
					fX[i][5] = ftmpBpz * fSVDscaling;
					fX[i][6] = 1.0F;
					i++;
				}
			}
		}
	}
	/* record how many measurement equations are available for the solution */
	nequations = i;

	/* call the SVD function and sort the eigenvalues and eigenvectors */
	SVDcompute(fX, nequations, 7, S, GkY);
	eigensort(S, GkY, 7);

	/* set ellipsoid matrix A to the solution vector column [6] with smallest eigenvalue */
	A[0][0] = GkY[0][6];
	A[1][1] = GkY[1][6];
	A[2][2] = GkY[2][6];
	A[0][1] = A[0][2] = A[1][0] = A[1][2] = A[2][0] = A[2][1] = 0.0F;

	/* compute the hard iron vector in offset bit counts times fSVDscaling */
	fVx = -0.5F * GkY[3][6] / A[0][0];
	fVy = -0.5F * GkY[4][6] / A[1][1];
	fVz = -0.5F * GkY[5][6] / A[2][2];

	/* negate A and gain if A has negative determinant. Sign change cancels for hard iron vector */
	det = A[0][0] * A[1][1] * A[2][2];
	if (det < 0.0F)
	{
		fmatrixAeqAxScalar(A, -1.0F, 3, 3);
		GkY[6][6] = -GkY[6][6];
		det = -det;
	}

	/* compute the geomagnetic field strength B in bit counts times fSVDscaling */
	fB = (float)sqrt(fabs(A[0][0] * fVx * fVx + A[1][1] * fVy * fVy + A[2][2] * fVz * fVz - GkY[6][6]));

	/* calculate the normalised fit error as a percentage */
	fFitErrorpc = S[6][0] / (2.0F * fB * fB * (float)sqrt((double) nequations)) * 100.0F;
	//printf("\nCalibration Fit Error (%%)=%9.3f", fFitErrorpc);

	/* correct for the measurement matrix offset and scaling and get the computed hard iron offset in uT */
	fVx = fVx * finvSVDscaling + fOffsetx;
	fVy = fVy * finvSVDscaling + fOffsety;
	fVz = fVz * finvSVDscaling + fOffsetz;
	//printf("\nCalibration hard iron (in uT) Vx=%9.3f Vy=%9.3f Vz=%9.3f", fVx, fVy, fVz);

	/* convert the geomagnetic field strength B into uT for current soft iron matrix A */
	fB *= finvSVDscaling;

	/* normalise the ellipsoid matrix A to unit determinant and correct B by root of this multiplicative factor */
	fmatrixAeqAxScalar(A, (float)pow((double)det, (double) (-1.0F / 3.0F)), 3, 3);
	fB *= (float)pow((double)det, (double) (-1.0F / 6.0F));
	//printf("\n\nCalibration geomagnetic field (uT) B=%9.3f", fB);

	/* compute invW from the square root of A also with normalised determinant */
	finvW[0][0] = (float)sqrt(fabs(A[0][0]));
	finvW[1][1] = (float)sqrt(fabs(A[1][1]));
	finvW[2][2] = (float)sqrt(fabs(A[2][2]));
	finvW[0][1] = finvW[0][2] = finvW[1][0] = finvW[1][2] = finvW[2][0] = finvW[2][1] = 0.0F;
	//printf("\n\nCalibration inverse soft iron matrix invW (normalized)");
	//fmatrixPrintA(finvW, 0, 2, 0, 2);

	/* for convenience show the original optimal invW */
	//printf("\nFor comparison: Simulation inverse soft iron matrix invW (normalized)");
	//fmatrixPrintA(invSimW, 0, 2, 0, 2);

	/* finally set the valid calibration flag to true */
	validcal = 1;
	if(( fp = fopen("/data/misc/sensors/g5s_calib", "rb+")) == NULL) {
	  printf("Cannot open file.\n");
	}
	if( fp != NULL) {
	  fwrite(&fVx, sizeof(float), 1, fp);
	  fwrite(&fVy, sizeof(float), 1, fp);
	  fwrite(&fVz, sizeof(float), 1, fp);
	}
	fclose(fp);
	return;
}

/* (FLOAT) 4 element calibration using 4x4 matrix inverse */
void fUpdateCalibration4INV(void)
{
	int i, j, k, l;                         /* loop counters */
	float fOffsetx, fOffsety, fOffsetz;     /* offset to remove large DC hard iron bias in matrix */
	float ftmpBpx, ftmpBpy, ftmpBpz;        /* scratch variables */

	printf("\n\nCalculating 4 element INV calibration at iteration %d with %d in Smart FIFO", loopcounter, ConstCount);

	/* the offsets are guaranteed to be set from the first element but to avoid compiler error */
	fOffsetx = fOffsety = fOffsetz = 0.0F;

	/* should never occur but return with no calibration if too few entries in the constellation */
	if (ConstCount < MINEQUATIONS)
	{
		return;
	}

	/* place the constellation sample number index into a temporary scratch array */
	i = 0;
	for (j = 0; j < CURRENTMAXPITCH; j++)
	{
		for (k = 0; k < CURRENTMAXROLL; k++)
		{
			for (l = 0; l < CURRENTMAXYAW; l++)
			{
				ScratchConIndex[i] = ConIndex[j][k][l];
				i++;
			}
		}
	}

	/* sort the scratch array into ascending order in place */
	heapSort(ScratchConIndex, (int)(CURRENTMAXPITCH * CURRENTMAXROLL * CURRENTMAXYAW));

	/* determine the earliest sample number that is needed to get MAXEQUATIONS: note this may be -1 */
	/* defensive programming test */
	if (MAXEQUATIONS <= CURRENTMAXPITCH * CURRENTMAXROLL * CURRENTMAXYAW)
	{
		earliestsample = ScratchConIndex[CURRENTMAXPITCH * CURRENTMAXROLL * CURRENTMAXYAW - MAXEQUATIONS];
	}
	else
	{
		earliestsample = 0;
	}

	/* place from MINEQUATIONS up to MAXEQUATIONS entries into the measurement matrix */
	i = 0;
	for (j = 0; j < CURRENTMAXPITCH; j++)
	{
		for (k = 0; k < CURRENTMAXROLL; k++)
		{
			for (l = 0; l < CURRENTMAXYAW; l++)
			{
				if ((ConIndex[j][k][l] != -1) && (ConIndex[j][k][l] >= earliestsample))
				{
					/* set tmp to valid data from the constellation */
					ftmpBpx = fConBpx[j][k][l];
					ftmpBpy = fConBpy[j][k][l];
					ftmpBpz = fConBpz[j][k][l];

					/* use first valid constellation entry as estimate (in uT) for offset to help solution */
					if (i == 0)
					{
						fOffsetx = ftmpBpx;
						fOffsety = ftmpBpy;
						fOffsetz = ftmpBpz;
					}

					/* apply the same fixed offset (in uT) to all measurement vectors for this iteration */
					ftmpBpx -= fOffsetx;
					ftmpBpy -= fOffsety;
					ftmpBpz -= fOffsetz;

					/* enter into the measurement matrix X scaling to make entries near unity */
					fX[i][0] = ftmpBpx * fSVDscaling;
					fX[i][1] = ftmpBpy * fSVDscaling;
					fX[i][2] = ftmpBpz * fSVDscaling;
					fX[i][3] = 1.0F;

					/* enter into dependent measurement vector Y */
					fY[i][0] = (ftmpBpx * ftmpBpx + ftmpBpy * ftmpBpy + ftmpBpz * ftmpBpz) * fSVDscaling * fSVDscaling;
					i++;
				}
			}
		}
	}
	/* record how many measurement equations are available for the solution */
	nequations = i;

	/* set inverse soft iron matrix invW to the identity matrix */
	finvW[0][0] = finvW[1][1] = finvW[2][2] = 1.0F;
	finvW[0][1] = finvW[0][2] = finvW[1][0] = finvW[1][2] = finvW[2][0] = finvW[2][1] = 0.0F;

	/* calculate Beta = Inv(X^T.X).X^T.Y */
	fmatrixAeqTrBxC(ftmpA4x4, fX, fX, nequations, 4, 4);	/* tmpA4x4 = X^T.X (4x4) */
	f4x4matrixAeqInvB(ftmpB4x4, ftmpA4x4);					/* tmpB4x4 = Inv(X^T.X) (4x4) */
	fmatrixAeqTrBxC(ftmpA4x1, fX, fY, nequations, 4, 1);	/* tmpA4x1 = X^T.Y (4x1) */
	fmatrixAeqBxC(fBeta4, ftmpB4x4, ftmpA4x1, 4, 4, 1);		/* Beta = Inv(X^T.X).X^T.Y (4x1) */ 

	/* calculate the performance function P = (Y-X.Beta)^T.(Y-X.Beta) */
	fmatrixAeqBxC(ftmpAMAXx1, fX, fBeta4, nequations, 4, 1);			/* tmpAMAXx1 = X.Beta (neqx1) */
	fFitErrorpc = 0.0F;
	for (i = 0; i < nequations; i++)
	{
		fFitErrorpc += (fY[i][0] - ftmpAMAXx1[i][0]) * (fY[i][0] - ftmpAMAXx1[i][0]);
	}

	/* calculate the Fit Error (percent) in scaled bit counts but not normalised by B */
	fFitErrorpc = (float)sqrt(fFitErrorpc / nequations) * 100.0F; 

	/* compute the hard iron vector in offset bit counts times fSVDscaling */
	fVx = 0.5F * fBeta4[0][0];
	fVy = 0.5F * fBeta4[1][0];
	fVz = 0.5F * fBeta4[2][0];

	/* compute the geomagnetic field strength B in bit counts times fSVDscaling */
	fB = (float)sqrt(fBeta4[3][0] + fVx * fVx + fVy * fVy + fVz * fVz);

	/* normalise the Fit Error (percent) to the scaled bit count geomagnetic field B */
	fFitErrorpc /= (2.0F * fB * fB); 
	printf("\nCalibration Fit Error (%%)=%9.3f", fFitErrorpc);

	/* correct for the measurement matrix offset and scaling and get the computed hard iron offset in uT */
	fVx = fVx * finvSVDscaling + fOffsetx;
	fVy = fVy * finvSVDscaling + fOffsety;
	fVz = fVz * finvSVDscaling + fOffsetz;
	printf("\nCalibration hard iron (uT) Vx=%9.3f Vy=%9.3f Vz=%9.3f", fVx, fVy, fVz);

	/* convert the geomagnetic field strength B into uT */
	fB *= finvSVDscaling;
	printf("\nCalibrated geomagnetic field (uT) B=%9.3f", fB);

	/* finally set the valid calibration flag to true */
	validcal = 1;

	return;
}

/* (FLOAT) calibration reset function */
void ResetCalibrationFunc(void)
{
	int j, k, l;   /* loop counters */ 

	/* initialise the calibration hard and soft iron estimate to null */
	fmatrixAeqI(finvW, 3);
	fVx = fVy = fVz = 0.;

	/* set the loop counter to 0 to denote first pass */
	loopcounter = 0;

	/* set valid calibration flag to false */
	validcal = 0;

	/* set constellation index to invalid value -1 to denote unfilled */
	for (j = 0; j < ABSMAXPITCH; j++)
		for (k = 0; k < ABSMAXROLL; k++)
			for (l = 0; l < ABSMAXYAW; l++)
				ConIndex[j][k][l] = -1;

	/* zero number of constellation bins filled */
	ConstCount = 0;

	return;
}

/* (FLOAT) function calculates the matrix product A = B x C */
void fmatrixAeqBxC(float **A, float **B, float **C, int rB, int cBrC, int cC)
{
	/* rB = rows in B */
	/* cBrC = columns in B = rows in C */
	/* cC = columns in C */
	/* A has dimension rB rows x cC columns */

	int i, j, k;	/* counters */

	for (i = 0; i < rB; i++)
	{
		for (j = 0; j < cC; j++)
		{
			A[i][j] = 0.0F;
			for (k = 0; k < cBrC; k++)
				A[i][j] += B[i][k] * C[k][j]; 
		}
	}
	return;
}

/* (FLOAT) function calculates the matrix product A = B^T x B */
void fmatrixAeqTrBxB(float **A, float **B, int r, int c)
{
	/* r = rows in B before transposing */
	/* c = columns in B before transposing */
	/* A has dimension c rows x c columns */

	int i, j, k;	/* counters */

	for (i = 0; i < c; i++)
	{
		for (j = 0; j < c; j++)
		{
			A[i][j] = 0.0F;
			for (k = 0; k < r; k++)
				A[i][j] += B[k][i] * B[k][j]; 
		}
	}
	return;
}

/* (FLOAT) function calculates the matrix product A = B^T x C */
void fmatrixAeqTrBxC(float **A,  float **B,  float **C, int rBrC, int cB, int cC)
{
	/* rBrC = rows in B before transposing = rows in C */
	/* cB = columns in B before transposing */
	/* cC = columns in C */
	/* A has dimension cB rows x cC columns */

	int i, j, k;	/* counters */

	for (i = 0; i < cB; i++)
	{
		for (j = 0; j < cC; j++)
		{
			A[i][j] = 0.0F;
			for (k = 0; k < rBrC; k++)
				A[i][j] += B[k][i] * C[k][j];
		}
	}
	return;
}

/* (FLOAT) function sets the matrix A to the identity matrix */
void fmatrixAeqI(float **A, int rc)
{
	/* rc = rows and columns in A */

	int i, j;		/* loop counters */

	for (i = 0; i < rc; i++)
	{
		for (j = 0; j < rc; j++)
		{
			A[i][j] = 0.0F;
		}
		A[i][i] = 1.0F;
	}
	return;
}

/* (FLOAT) function multiplies all elements of matrix A by the specified scalar */
void fmatrixAeqAxScalar(float **A, float Scalar, int r, int c)
{
	/* r = rows and c = columns in A */

	int i, j;		/* loop counters */

	for (i = 0; i < r; i++)
	{
		for (j = 0; j < c; j++)
		{
			A[i][j] *= Scalar;
		}
	}
	return;
}

/* (FLOAT) function prints the floating point matrix A between rows r1 and r2 and columns c1 and c2 inclusve */
void fmatrixPrintA(float **A, int r1, int r2, int c1, int c2)
{
	int i, j;		/* loop counters */

	for (i = r1; i <= r2; i++)
	{
		printf("\nRow %d", i);
		for (j = c1; j <= c2; j++)
		{
			printf("%12.5f", (double) A[i][j]);
		}
	}
	return;
}

/* (FLOAT) function calculates the determinant of a 3x3 matrix */
float f3x3matrixDetA(float **A)
{
	float det;		/* determinant */

	det = A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1])
		+ A[0][1] * (A[1][2] * A[2][0] - A[1][0] * A[2][2]) 
		+ A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);

	return(det);
}

/* (FLOAT) function calculates the inverse of a 3x3 matrix */
void f3x3matrixAeqInvB(float **A, float **B)
{
	float ftmp;			/* determinant and then reciprocal */

	ftmp = B[0][0] * (B[1][1] * B[2][2] - B[1][2] * B[2][1])
		+ B[0][1] * (B[1][2] * B[2][0] - B[1][0] * B[2][2]) 
		+ B[0][2] * (B[1][0] * B[2][1] - B[1][1] * B[2][0]);

	/* compute inverse for any determinant except zero */
	if (ftmp != 0.0F)
	{
		ftmp = 1.0F / ftmp;
		A[0][0] = (B[1][1] * B[2][2] - B[2][1] * B[1][2]) * ftmp;
		A[0][1] = (B[0][2] * B[2][1] - B[0][1] * B[2][2]) * ftmp;
		A[0][2] = (B[0][1] * B[1][2] - B[0][2] * B[1][1]) * ftmp;
		A[1][0] = (B[1][2] * B[2][0] - B[1][0] * B[2][2]) * ftmp;
		A[1][1] = (B[0][0] * B[2][2] - B[0][2] * B[2][0]) * ftmp;
		A[1][2] = (B[0][2] * B[1][0] - B[0][0] * B[1][2]) * ftmp;
		A[2][0] = (B[1][0] * B[2][1] - B[2][0] * B[1][1]) * ftmp;
		A[2][1] = (B[0][1] * B[2][0] - B[0][0] * B[2][1]) * ftmp;
		A[2][2] = (B[0][0] * B[1][1] - B[0][1] * B[1][0]) * ftmp;
	}
	else
	{
		/* provide the identity matrix if the determinant is zero */
		printf("\nZero determinant detected in f3x3matrixAeqInvB");
		A[0][0] = A[1][1] = A[2][2] = 1.0F; 
		A[0][1] = A[0][2] = A[1][0] = A[1][2] = A[2][0] = A[2][1] = 0.0F;
	}
	return;
}

/* (FLOAT) function calculates the inverse of a 4x4 matrix by brute force */
void f4x4matrixAeqInvB(float **A, float **B)
{
	float ftmp;			/* determinant and then reciprocal */
	int i, j;			/* loop counters */

	ftmp = B[0][3]*B[1][2]*B[2][1]*B[3][0] - B[0][2]*B[1][3]*B[2][1]*B[3][0] - 
		B[0][3]*B[1][1]*B[2][2]*B[3][0] + B[0][1]*B[1][3]*B[2][2]*B[3][0] +
		B[0][2]*B[1][1]*B[2][3]*B[3][0] - B[0][1]*B[1][2]*B[2][3]*B[3][0] - 
		B[0][3]*B[1][2]*B[2][0]*B[3][1] + B[0][2]*B[1][3]*B[2][0]*B[3][1] +
		B[0][3]*B[1][0]*B[2][2]*B[3][1] - B[0][0]*B[1][3]*B[2][2]*B[3][1] - 
		B[0][2]*B[1][0]*B[2][3]*B[3][1] + B[0][0]*B[1][2]*B[2][3]*B[3][1] +
		B[0][3]*B[1][1]*B[2][0]*B[3][2] - B[0][1]*B[1][3]*B[2][0]*B[3][2] - 
		B[0][3]*B[1][0]*B[2][1]*B[3][2] + B[0][0]*B[1][3]*B[2][1]*B[3][2] +
		B[0][1]*B[1][0]*B[2][3]*B[3][2] - B[0][0]*B[1][1]*B[2][3]*B[3][2] - 
		B[0][2]*B[1][1]*B[2][0]*B[3][3] + B[0][1]*B[1][2]*B[2][0]*B[3][3] +
		B[0][2]*B[1][0]*B[2][1]*B[3][3] - B[0][0]*B[1][2]*B[2][1]*B[3][3] - 
		B[0][1]*B[1][0]*B[2][2]*B[3][3] + B[0][0]*B[1][1]*B[2][2]*B[3][3];

	/* compute inverse for any determinant except zero */
	if (ftmp != 0.0F)
	{
		ftmp = 1.0F / ftmp;
		A[0][0] = (B[1][2]*B[2][3]*B[3][1] - B[1][3]*B[2][2]*B[3][1] + B[1][3]*B[2][1]*B[3][2] -
			B[1][1]*B[2][3]*B[3][2] - B[1][2]*B[2][1]*B[3][3] + B[1][1]*B[2][2]*B[3][3]) * ftmp;
		A[0][1] = (B[0][3]*B[2][2]*B[3][1] - B[0][2]*B[2][3]*B[3][1] - B[0][3]*B[2][1]*B[3][2] +
			B[0][1]*B[2][3]*B[3][2] + B[0][2]*B[2][1]*B[3][3] - B[0][1]*B[2][2]*B[3][3]) * ftmp;
		A[0][2] = (B[0][2]*B[1][3]*B[3][1] - B[0][3]*B[1][2]*B[3][1] + B[0][3]*B[1][1]*B[3][2] - 
			B[0][1]*B[1][3]*B[3][2] - B[0][2]*B[1][1]*B[3][3] + B[0][1]*B[1][2]*B[3][3]) * ftmp;
		A[0][3] = (B[0][3]*B[1][2]*B[2][1] - B[0][2]*B[1][3]*B[2][1] - B[0][3]*B[1][1]*B[2][2] +
			B[0][1]*B[1][3]*B[2][2] + B[0][2]*B[1][1]*B[2][3] - B[0][1]*B[1][2]*B[2][3]) * ftmp;
		A[1][0] = (B[1][3]*B[2][2]*B[3][0] - B[1][2]*B[2][3]*B[3][0] - B[1][3]*B[2][0]*B[3][2] +
			B[1][0]*B[2][3]*B[3][2] + B[1][2]*B[2][0]*B[3][3] - B[1][0]*B[2][2]*B[3][3]) * ftmp;
		A[1][1] = (B[0][2]*B[2][3]*B[3][0] - B[0][3]*B[2][2]*B[3][0] + B[0][3]*B[2][0]*B[3][2] - 
			B[0][0]*B[2][3]*B[3][2] - B[0][2]*B[2][0]*B[3][3] + B[0][0]*B[2][2]*B[3][3]) * ftmp;
		A[1][2] = (B[0][3]*B[1][2]*B[3][0] - B[0][2]*B[1][3]*B[3][0] - B[0][3]*B[1][0]*B[3][2] + 
			B[0][0]*B[1][3]*B[3][2] + B[0][2]*B[1][0]*B[3][3] - B[0][0]*B[1][2]*B[3][3]) * ftmp;
		A[1][3] = (B[0][2]*B[1][3]*B[2][0] - B[0][3]*B[1][2]*B[2][0] + B[0][3]*B[1][0]*B[2][2] - 
			B[0][0]*B[1][3]*B[2][2] - B[0][2]*B[1][0]*B[2][3] + B[0][0]*B[1][2]*B[2][3]) * ftmp;
		A[2][0] = (B[1][1]*B[2][3]*B[3][0] - B[1][3]*B[2][1]*B[3][0] + B[1][3]*B[2][0]*B[3][1] - 
			B[1][0]*B[2][3]*B[3][1] - B[1][1]*B[2][0]*B[3][3] + B[1][0]*B[2][1]*B[3][3]) * ftmp;
		A[2][1] = (B[0][3]*B[2][1]*B[3][0] - B[0][1]*B[2][3]*B[3][0] - B[0][3]*B[2][0]*B[3][1] + 
			B[0][0]*B[2][3]*B[3][1] + B[0][1]*B[2][0]*B[3][3] - B[0][0]*B[2][1]*B[3][3]) * ftmp;
		A[2][2] = (B[0][1]*B[1][3]*B[3][0] - B[0][3]*B[1][1]*B[3][0] + B[0][3]*B[1][0]*B[3][1] - 
			B[0][0]*B[1][3]*B[3][1] - B[0][1]*B[1][0]*B[3][3] + B[0][0]*B[1][1]*B[3][3]) * ftmp;
		A[2][3] = (B[0][3]*B[1][1]*B[2][0] - B[0][1]*B[1][3]*B[2][0] - B[0][3]*B[1][0]*B[2][1] + 
			B[0][0]*B[1][3]*B[2][1] + B[0][1]*B[1][0]*B[2][3] - B[0][0]*B[1][1]*B[2][3]) * ftmp;
		A[3][0] = (B[1][2]*B[2][1]*B[3][0] - B[1][1]*B[2][2]*B[3][0] - B[1][2]*B[2][0]*B[3][1] + 
			B[1][0]*B[2][2]*B[3][1] + B[1][1]*B[2][0]*B[3][2] - B[1][0]*B[2][1]*B[3][2]) * ftmp;
		A[3][1] = (B[0][1]*B[2][2]*B[3][0] - B[0][2]*B[2][1]*B[3][0] + B[0][2]*B[2][0]*B[3][1] - 
			B[0][0]*B[2][2]*B[3][1] - B[0][1]*B[2][0]*B[3][2] + B[0][0]*B[2][1]*B[3][2]) * ftmp;
		A[3][2] = (B[0][2]*B[1][1]*B[3][0] - B[0][1]*B[1][2]*B[3][0] - B[0][2]*B[1][0]*B[3][1] + 
			B[0][0]*B[1][2]*B[3][1] + B[0][1]*B[1][0]*B[3][2] - B[0][0]*B[1][1]*B[3][2]) * ftmp;
		A[3][3] = (B[0][1]*B[1][2]*B[2][0] - B[0][2]*B[1][1]*B[2][0] + B[0][2]*B[1][0]*B[2][1] - 
			B[0][0]*B[1][2]*B[2][1] - B[0][1]*B[1][0]*B[2][2] + B[0][0]*B[1][1]*B[2][2]) * ftmp;
	}
	else
	{
		/* provide the identity matrix if the determinant is zero */
		printf("\nZero determinant detected in f4x4matrixAeqInvB");
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 4; j++)
			{
				A[i][j] = 0.0F;
			}
			A[i][i] = 1.0F;
		}
	}

	return;
}

/* (FLOAT) 6DOF sensor driver simulation */
void fSixDOFSensorDrivers(int k)
{
	// accelerometer
	int res;
	int ioctl_msg=0;
	if (aflag==1 || oflag==1)
	{
	if (prev_aflag==0 && prev_oflag==0) {
		fd_bma = open("/dev/bma_accel", O_RDWR); }
	short acc[3]={0};
	ioctl_msg=BMA220_READ_ACCEL_XYZ;
	res = ioctl(fd_bma, ioctl_msg, &acc);
	fGpx = (float)acc[0]/256;
	fGpy = (float)acc[1]/256;
	fGpz = (float)acc[2]/256;
	}

	// magnetometer
	if (mflag==1 || oflag==1)
	{
	if (prev_mflag==0 && prev_oflag==0) {
		fd_mmc = open("/dev/mmc31xx", O_RDWR); }
	int tmag[3]={0};
	int mag[3]={0};
	ioctl_msg=MMC31XX_IOC_READXYZ;
	res = ioctl(fd_mmc, ioctl_msg, &tmag);
	mag[0]=tmag[0]-4096;
	mag[1]=tmag[1]-4096;
	mag[2]=tmag[2]-4096;
	fBpx = (float)mag[0]/10;
	fBpy = (float)mag[1]/10;
	fBpz = (float)mag[2]/10;
	}
	//printf("fGpx %f, fGpy %f, fGpz %f, fBpx %f, fBpy %f, fBpz %f\n", fGpx, fGpy, fGpz, fBpx, fBpy, fBpz);
	
	// close sensors if possible
	if (prev_aflag==1 || prev_oflag==1) {
		if (aflag==0 && oflag==0) close(fd_bma);
	}
	if (prev_mflag==1 || prev_oflag==1) {
		if (mflag==0 && oflag==0) close(fd_mmc);
	}

	// store current sensor state for next iteration
	prev_aflag = aflag;
	prev_mflag = mflag;
	prev_oflag = oflag;
	return;
} 

/* (FLOAT) sorts the eigenvalues eigval and eigenvectors eigvec from eigencompute into descending order */
void eigensort(float **eigval, float **eigvec, int n)
{
	int k, j, i;
	float p;

	for (i = 0; i < n - 1; i++)
	{
		p = eigval[k = i][0];
		for (j = i + 1; j < n; j++)
			if (eigval[j][0] >= p) 
			{
				p = eigval[k = j][0];
			}
			if (k != i)
			{
				eigval[k][0] = eigval[i][0];
				eigval[i][0] = p;
				for (j = 0; j < n; j++)
				{
					p = eigvec[j][i];
					eigvec[j][i] = eigvec[j][k];
					eigvec[j][k] = p;
				}
			}
	}
	return;
}

/* macro definitions for svdcompute */
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

static float sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg * sqrarg)

static float maxarg1, maxarg2;
#define FMAX(a,b) (maxarg1=(a), maxarg2=(b), (maxarg1) > (maxarg2) ? (maxarg1) : (maxarg2))

static int iminarg1, iminarg2;
#define IMIN(a,b) (iminarg1=(a), iminarg2=(b), (iminarg1) < (iminarg2) ? (iminarg1) : (iminarg2))

/* compute (a^2 + b^2)^1/2 without destructive underflow or overflow */
float pythag(float a, float b)
{
	float absa, absb;
	absa = fabs(a);
	absb = fabs(b);
	if (absa > absb) 
	{
		return (float) (absa * sqrt(1.0F + SQR(absb / absa)));
	}
	else 
	{
		return (absb == 0.0F ? (float) 0.0F : (float) (absb * sqrt(1.0F + SQR(absa / absb))));
	}
}

/******************************************************************************/
void SVDcompute(float **mat, int m, int n, float **w, float **v)
/*******************************************************************************
	function computes the singular value decomposition of matrix mat[0..m-1][0..n-1]
	mat = u.w.v^T.
	The matrix u replaces mat on output.
	The diagonal matrix of singular values w is output as a vector w[0..n-1].
	The matrix v (not the transpose v^T) is output as v[0..n-1][0..n-1].
	The columns and rows of v are orthonormal
	The eigenvalues and eigenvectors are not sorted.
	This implementation based on code published by University of Western Australia:
	http://www.csse.uwa.edu.au/pub/robvis/theses/BruceBackman/code_and_data/svd/svdcmp.c
	*******************************************************************************/
{
	int flag, i, its, j, jj, k, l, nm;
	float anorm, c, f, g, h, s, scale, x, y, z;
	float tmpv[MAXCALELEMENTS];	

	g = scale = anorm = 0.0F;
	for (i = 0; i < n; i++) 
	{
		l = i + 1;
		tmpv[i] = scale * g;
		g = s = scale = 0.0F;
		if (i < m) 
		{
			for (k = i; k < m; k++) 
			{
				scale += fabs(mat[k][i]);
			}
			if (scale != 0.0F) 
			{
				for (k = i; k < m; k++) 
				{
					mat[k][i] /= scale;
					s += mat[k][i] * mat[k][i];
				}
				f = mat[i][i];
				g = -SIGN(sqrt(s), f);
				h = f * g - s;
				mat[i][i] = f - g; 
				for (j = l; j < n; j++)
				{
					for (s = 0.0F, k = i; k < m; k++) 
					{
						s += mat[k][i] * mat[k][j];
					}
					f = s / h;
					for (k = i; k < m; k++)
					{
						mat[k][j] += f * mat[k][i];
					}
				}
				for (k = i; k < m; k++) 
				{
					mat[k][i] *= scale;
				}
			}
		}
		w[i][0] = scale * g;
		g = s = scale = 0.0F;
		if ((i < m) && (i != (n - 1)))
		{
			for (k = l; k < n; k++) 
			{
				scale += fabs(mat[i][k]);
			}
			if (scale != 0.0F) 
			{
				for (k = l; k < n; k++) 
				{
					mat[i][k] /= scale;
					s += mat[i][k] * mat[i][k];
				}
				f = mat[i][l];
				g = -SIGN(sqrt(s), f);
				h = f * g - s;
				mat[i][l] = f - g;
				for (k = l; k < n; k++)
				{
					tmpv[k] = mat[i][k] / h;
				}
				for (j = l; j < m; j++) 
				{
					for (s = 0.0F, k = l; k < n; k++) 
					{
						s += mat[j][k] * mat[i][k];
					}
					for (k = l; k < n; k++) 
					{
						mat[j][k] += s * tmpv[k];
					}
				}
				for (k = l; k < n; k++) 
				{
					mat[i][k] *= scale;
				}
			}
		}
		anorm = FMAX(anorm, (fabs(w[i][0]) + fabs(tmpv[i])));
	}
	for (i = n - 1; i >= 0; i--)
	{ 
		if (i < (n - 1)) 
		{
			if (g != 0.0F) 
			{
				for (j = l; j < n; j++) 
				{
					v[j][i] = (mat[i][j] / mat[i][l]) / g;
				}
				for (j = l; j < n; j++) 
				{
					for (s = 0.0F, k = l; k < n; k++)
					{ 
						s += mat[i][k] * v[k][j];
					}
					for (k = l; k < n; k++)
					{ 
						v[k][j] += s * v[k][i];
					}
				}
			}
			for (j = l; j < n; j++)
			{
				v[i][j] = v[j][i] = 0.0F;
			}
		}
		v[i][i] = 1.0F;
		g = tmpv[i];
		l = i;
	}
	for (i = IMIN((m - 1), (n - 1)); i >= 0; i--) 
	{ 
		l = i + 1;
		g = w[i][0];
		for (j = l; j < n; j++)
		{ 
			mat[i][j] = 0.0F;
		}
		if (g != 0.0F) 
		{
			g = 1.0F / g;
			for (j = l; j < n; j++) 
			{
				for (s = 0.0F, k = l; k < m; k++)
				{
					s += mat[k][i] * mat[k][j];
				}
				f = (s / mat[i][i]) * g;
				for (k = i; k < m; k++)
				{
					mat[k][j] += f * mat[k][i];
				}
			}
			for (j = i; j < m; j++) 
			{
				mat[j][i] *= g;
			}
		} 
		else 
		{
			for (j = i; j < m; j++)
			{
				mat[j][i] = 0.0F;
			}
		}
		++mat[i][i];
	}
	for (k = n - 1; k >= 0; k--) 
	{ 
		for (its = 1; its <= 30; its++) 
		{
			flag = 1;
			for (l = k; l >= 0; l--) 
			{ 
				nm = l - 1; 
				if ((float)(fabs(tmpv[l]) + anorm) == anorm) 
				{
					flag = 0;
					break;
				}
				if ((float)(fabs(w[nm][0]) + anorm) == anorm)
				{
					break;
				}
			}
			if (flag) 
			{
				c = 0.0F; 
				s = 1.0F;
				for (i = l; i <= k; i++) 
				{
					f = s * tmpv[i];
					tmpv[i] = c * tmpv[i];
					if ((float)(fabs(f) + anorm) == anorm)
					{
						break;
					}
					g = w[i][0];
					h = pythag(f, g);
					w[i][0] = h;
					h = 1.0F / h;
					c = g * h;
					s = -f * h;
					for (j = 0; j < m; j++)
					{
						y = mat[j][nm];
						z = mat[j][i];
						mat[j][nm] = y * c + z * s;
						mat[j][i] = z * c - y * s;
					}
				}
			}
			z = w[k][0];
			if (l == k) 
			{ 
				if (z < 0.0F) 
				{ 
					w[k][0] = -z;
					for (j = 0; j < n; j++) 
					{
						v[j][k] = -v[j][k];
					}
				}
				break;
			}
			if (its == 30) 
			{
				printf("\nNo convergence in 30 SVDcompute iterations");
			}
			x = w[l][0]; 
			nm = k - 1;
			y = w[nm][0];
			g = tmpv[nm];
			h = tmpv[k];
			f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0F * h * y);
			g = pythag(f, 1.0F);
			f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
			c = s = 1.0F; 
			for (j = l; j <= nm; j++) 
			{
				i = j + 1;
				g = tmpv[i]; 
				y = w[i][0];
				h = s * g;
				g = c * g;
				z = pythag(f, h);
				tmpv[j] = z;
				c = f / z;
				s = h / z;
				f = x * c + g * s;
				g = g * c - x * s;
				h = y * s;
				y *= c;
				for (jj = 0; jj < n; jj++) 
				{
					x = v[jj][j];
					z = v[jj][i];
					v[jj][j] = x * c + z * s;
					v[jj][i] = z * c - x * s;
				}
				z = pythag(f, h);
				w[j][0] = z; 
				if (z) 
				{
					z = 1.0F / z;
					c = f * z;
					s = h * z;
				}
				f = c * g + s * y;
				x = c * y - s * g;
				for (jj = 0; jj < m; jj++)
				{
					y = mat[jj][j];
					z = mat[jj][i];
					mat[jj][j] = y * c + z * s;
					mat[jj][i] = z * c - y * s;
				}
			}
			tmpv[l] = 0.0F;
			tmpv[k] = f;
			w[k][0] = x;
		}
	}
	return;
}

/* heapsort algorithm */
void heapSort(int numbers[], int array_size)
{
	int i, temp;

	for (i = (array_size / 2); i >= 0; i--)
		siftDown(numbers, i, array_size - 1);

	for (i = array_size - 1; i >= 1; i--)
	{
		temp = numbers[0];
		numbers[0] = numbers[i];
		numbers[i] = temp;
		siftDown(numbers, 0, i - 1);
	}
	return;
}

/* function called by heapsort */
void siftDown(int numbers[], int root, int bottom)
{
	int maxChild, temp;
	bool done;

	done = false;
	while ((root * 2 <= bottom) && (!done))
	{
		if (root * 2 == bottom)
			maxChild = root * 2;
		else if (numbers[root * 2] > numbers[root * 2 + 1])
			maxChild = root * 2;
		else
			maxChild = root * 2 + 1;

		if (numbers[root] < numbers[maxChild])
		{
			temp = numbers[root];
			numbers[root] = numbers[maxChild];
			numbers[maxChild] = temp;
			root = maxChild;
		}
		else
			done = true;
	}
	return;
}
