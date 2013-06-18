#define F_CPU 16000000UL //16mhz
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "TouchScreen.h" 
#include "TFT.h"
#include "config.h"
#include "twicam.h"
#include "captureimage.h"
#include "camregdef.h"
//TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
uint8_t buf[640];
#ifndef MT9D111
void menu(void);
uint16_t leadingZeros(uint8_t x);
void setmatrix(uint8_t id)
{
	switch (id) {
		case 0:
			wrReg(MTX1,0x80);        
			wrReg(MTX2,0x80);      
			wrReg(MTX3,0x00);        
			wrReg(MTX4,0x22);        
			wrReg(MTX5,0x5e);        
			wrReg(MTX6,0x80);        
			wrReg(MTXS,0x9e); 
		break;
		case 1:
			wrReg(MTX1,0x40);        
			wrReg(MTX2,0x34);      
			wrReg(MTX3,0x0c);        
			wrReg(MTX4,0x17);
			wrReg(MTX5,0x29);        
			wrReg(MTX6,0x40);
			wrReg(MTXS,0x1A);
		break;
		case 2:
			wrReg(MTX1,0xB3);        
			wrReg(MTX2,0xB3);      
			wrReg(MTX3,0x0);        
			wrReg(MTX4,0x3D);        
			wrReg(MTX5,0xA7);        
			wrReg(MTX6,0xE4);
			wrReg(MTXS,0x9E);
		break;
	}
}
const struct regval_list ov7670_default_regs[] PROGMEM = {
	{ REG_COM7, COM7_RESET },
	{ REG_TSLB,  0x04 },	/* OV */
	{ REG_COM7, 0 },	/* VGA */
	/*
	 * Set the hardware window.  These values from OV don't entirely
	 * make sense - hstop is less than hstart.  But they work...
	 */
	{ REG_HSTART, 0x13 },	{ REG_HSTOP, 0x01 },
	{ REG_HREF, 0xb6 },	{ REG_VSTART, 0x02 },
	{ REG_VSTOP, 0x7a },	{ REG_VREF, 0x0a },

	{ REG_COM3, 0 },	{ REG_COM14, 0 },
	/* Mystery scaling numbers */
	{ 0x70, 0x3a },		{ 0x71, 0x35 },
	{ 0x72, 0x11 },		{ 0x73, 0xf0 },
	{ 0xa2,/* 0x02 changed to 1*/1},{ REG_COM10, 0x0 },
	/* Gamma curve values */
	{ 0x7a, 0x20 },		{ 0x7b, 0x10 },
	{ 0x7c, 0x1e },		{ 0x7d, 0x35 },
	{ 0x7e, 0x5a },		{ 0x7f, 0x69 },
	{ 0x80, 0x76 },		{ 0x81, 0x80 },
	{ 0x82, 0x88 },		{ 0x83, 0x8f },
	{ 0x84, 0x96 },		{ 0x85, 0xa3 },
	{ 0x86, 0xaf },		{ 0x87, 0xc4 },
	{ 0x88, 0xd7 },		{ 0x89, 0xe8 },
	/* AGC and AEC parameters.  Note we start by disabling those features,
	   then turn them only after tweaking the values. */
	{ REG_COM8, COM8_FASTAEC | COM8_AECSTEP | COM8_BFILT },
	{ REG_GAIN, 0 },	{ REG_AECH, 0 },
	{ REG_COM4, 0x40 }, /* magic reserved bit */
	{ REG_COM9, 0x18 }, /* 4x gain + magic rsvd bit */
	{ REG_BD50MAX, 0x05 },	{ REG_BD60MAX, 0x07 },
	{ REG_AEW, 0x95 },	{ REG_AEB, 0x33 },
	{ REG_VPT, 0xe3 },	{ REG_HAECC1, 0x78 },
	{ REG_HAECC2, 0x68 },	{ 0xa1, 0x03 }, /* magic */
	{ REG_HAECC3, 0xd8 },	{ REG_HAECC4, 0xd8 },
	{ REG_HAECC5, 0xf0 },	{ REG_HAECC6, 0x90 },
	{ REG_HAECC7, 0x94 },
	{ REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC },
        {0x30,0},{0x31,0},//disable some delays
	/* Almost all of these are magic "reserved" values.  */
	{ REG_COM5, 0x61 },	{ REG_COM6, 0x4b },
	{ 0x16, 0x02 },		{ REG_MVFP, 0x07 },
	{ 0x21, 0x02 },		{ 0x22, 0x91 },
	{ 0x29, 0x07 },		{ 0x33, 0x0b },
	{ 0x35, 0x0b },		{ 0x37, 0x1d },
	{ 0x38, 0x71 },		{ 0x39, 0x2a },
	{ REG_COM12, 0x78 },	{ 0x4d, 0x40 },
	{ 0x4e, 0x20 },		{ REG_GFIX, 0 },
	/*{ 0x6b, 0x4a },*/		{ 0x74, 0x10 },
	{ 0x8d, 0x4f },		{ 0x8e, 0 },
	{ 0x8f, 0 },		{ 0x90, 0 },
	{ 0x91, 0 },		{ 0x96, 0 },
	{ 0x9a, 0 },		{ 0xb0, 0x84 },
	{ 0xb1, 0x0c },		{ 0xb2, 0x0e },
	{ 0xb3, 0x82 },		{ 0xb8, 0x0a },

	/* More reserved magic, some of which tweaks white balance */
	{ 0x43, 0x0a },		{ 0x44, 0xf0 },
	{ 0x45, 0x34 },		{ 0x46, 0x58 },
	{ 0x47, 0x28 },		{ 0x48, 0x3a },
	{ 0x59, 0x88 },		{ 0x5a, 0x88 },
	{ 0x5b, 0x44 },		{ 0x5c, 0x67 },
	{ 0x5d, 0x49 },		{ 0x5e, 0x0e },
	{ 0x6c, 0x0a },		{ 0x6d, 0x55 },
	{ 0x6e, 0x11 },		{ 0x6f, 0x9e }, /* it was 0x9F "9e for advance AWB" */
	{ 0x6a, 0x40 },		{ REG_BLUE, 0x40 },
	{ REG_RED, 0x60 },
	{ REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC|COM8_AWB },

	/* Matrix coefficients */
	{ 0x4f, 0x80 },		{ 0x50, 0x80 },
	{ 0x51, 0 },		{ 0x52, 0x22 },
	{ 0x53, 0x5e },		{ 0x54, 0x80 },
	{ 0x58, 0x9e },

	{ REG_COM16, COM16_AWBGAIN },	{ REG_EDGE, 0 },
	{ 0x75, 0x05 },		{ 0x76, 0xe1 },
	{ 0x4c, 0 },		{ 0x77, 0x01 },
	{ REG_COM13, 0xc3 },	{ 0x4b, 0x09 },
	{ 0xc9, 0x60 },		/*{ REG_COM16, 0x38 },*/
	{ 0x56, 0x40 },

	{ 0x34, 0x11 },		{ REG_COM11, COM11_EXP|COM11_HZAUTO },
	{ 0xa4, 0x82/*Wax0x88*/ },		{ 0x96, 0 },
	{ 0x97, 0x30 },		{ 0x98, 0x20 },
	{ 0x99, 0x30 },		{ 0x9a, 0x84 },
	{ 0x9b, 0x29 },		{ 0x9c, 0x03 },
	{ 0x9d, 0x4c },		{ 0x9e, 0x3f },
	{ 0x78, 0x04 },

	/* Extra-weird stuff.  Some sort of multiplexor register */
	{ 0x79, 0x01 },		{ 0xc8, 0xf0 },
	{ 0x79, 0x0f },		{ 0xc8, 0x00 },
	{ 0x79, 0x10 },		{ 0xc8, 0x7e },
	{ 0x79, 0x0a },		{ 0xc8, 0x80 },
	{ 0x79, 0x0b },		{ 0xc8, 0x01 },
	{ 0x79, 0x0c },		{ 0xc8, 0x0f },
	{ 0x79, 0x0d },		{ 0xc8, 0x20 },
	{ 0x79, 0x09 },		{ 0xc8, 0x80 },
	{ 0x79, 0x02 },		{ 0xc8, 0xc0 },
	{ 0x79, 0x03 },		{ 0xc8, 0x40 },
	{ 0x79, 0x05 },		{ 0xc8, 0x30 },
	{ 0x79, 0x26 },
	
	{ 0xff, 0xff },	/* END MARKER */
};
#endif
//undocumented rgister 1F is called LAEC this means exposures less than one line
void initCam(void)
{
  #ifdef MT9D111
	_delay_ms(100);
  #else
	wrReg(0x12, 0x80);
	_delay_ms(100);
	wrSensorRegs8_8(ov7670_default_regs);
	// _delay_ms(1000);
	wrReg(REG_COM10,32);//pclk does not toggle on HBLANK
	//wrReg(0x6B,74);
	//wrReg(0x11,32);//using scaler for divider
	wrReg(REG_RGB444, 0x00);             // Disable RGB444
	wrReg(REG_COM11,226);//enable night mode 1/8 frame rate COM11
	//wrReg(0x2E,63);
	wrReg(REG_TSLB,0x04);                // 0D = UYVY  04 = YUYV     
 	wrReg(REG_COM13,0xC8);               // connect to REG_TSLB
	//wrReg(REG_COM13,0x8);               // connect to REG_TSLB disable gamma
	#ifdef rgb565
	wrReg(REG_COM7, 0x04);           // RGB
	wrReg(REG_COM15, 0xD0);          // Set rgb565 with Full range    0xD0
	#elif defined rawRGB
	wrReg(REG_COM7,1);//raw rgb bayer
	wrReg(REG_COM15, 0xC0);          //Full range
	wrReg(REG_REG76,0xe1);/* Pix correction, magic rsvd */
	#else
	wrReg(REG_COM7, 0x00);           // YUV
	//wrReg(REG_COM17, 0x00);          // color bar disable
	wrReg(REG_COM15, 0xC0);          //Full range
	#endif
	//wrReg(REG_COM3, 0x04);
	#if defined qqvga || defined qvga
	wrReg(REG_COM3,4);    // REG_COM3 enable scaling
	#else
	wrReg(REG_COM3,0);    // REG_COM3
	#endif
	//wrReg(0x3e,0x00);        //  REG_COM14
	//   wrReg(0x72,0x11);        //
	// wrReg(0x73,0xf0);        //
	//wrReg(REG_COM8,0x8F);        // AGC AWB AEC Unlimited step size
	wrReg(REG_COM8,0xCF);//disable AGC disable AEC
	//  wrReg(REG_COM1, 3);//manual exposure
	// wrReg(0x07, 0xFF);//manual exposure
	//wrReg(0x10, 0xFF);//manual exposure*/
	#ifdef qqvga
	wrReg(REG_COM14, 0x1a);          // divide by 4
        wrReg(0x72, 0x22);               // downsample by 4
        wrReg(0x73, 0xf2);               // divide by 4
        wrReg(REG_HSTART,0x16);
        wrReg(REG_HSTOP,0x04);
        wrReg(REG_HREF,0xa4);           
        wrReg(REG_VSTART,0x02);
        wrReg(REG_VSTOP,0x7a);
        wrReg(REG_VREF,0x0a);
	#endif
	#ifdef qvga
	wrReg(REG_COM14, 0x19);//0x19
        wrReg(0x72, 0x11);
        wrReg(0x73, 0xf1);//f1
        wrReg(REG_HSTART,0x16);
        wrReg(REG_HSTOP,0x04);
        wrReg(REG_HREF,0x24);
        wrReg(REG_VSTART,0x02);
        wrReg(REG_VSTOP,0x7a);
        wrReg(REG_VREF,0x0a);
	#else
        wrReg(REG_HREF,0xF6);        // was B6  
        wrReg(0x17,0x13);        // HSTART
        wrReg(0x18,0x01);        // HSTOP
        wrReg(0x19,0x02);        // VSTART
        wrReg(0x1a,0x7a);        // VSTOP
        wrReg(REG_VREF,0x0a);        // VREF
	//wrReg(REG_VREF,0xCA);//set 2 high GAIN MSB
	#endif
       // wrReg(0x70, 0x3a);       // Scaling Xsc
        //wrReg(0x71, 0x35);       // Scaling Ysc
        //wrReg(0xA2, 0x02);       // pixel clock delay
 
  // COLOR SETTING
//	wrReg(0,0xFF);//set gain to maxium possile
    wrReg(0xAA,0x94);            // Average-based AEC algorithm
    wrReg(REG_BRIGHT,0x00);      // 0x00(Brightness 0) - 0x18(Brightness +1) - 0x98(Brightness -1)
    wrReg(REG_CONTRAS,0x40);     // 0x40(Contrast 0) - 0x50(Contrast +1) - 0x38(Contrast -1)
    //wrReg(0xB1,0xB1);            // Automatic Black level Calibration
    wrReg(0xb1,4);//really enable ABLC
	setmatrix(3);  
	/*wrReg(0x4f, 0xb3); 	// "matrix coefficient 1" 
	wrReg(0x50, 0xb3); 	// "matrix coefficient 2"
	wrReg( 0x51, 0);		// vb
	wrReg(0x52, 0x3d ); 	// "matrix coefficient 4"
	wrReg(0x53, 0xa7 ); 	// "matrix coefficient 5" 
	wrReg(0x54, 0xe4 ); 	// "matrix coefficient 6"*/
    wrReg(AWBC7,0x88);
    wrReg(AWBC8,0x88);
    wrReg(AWBC9,0x44);
    wrReg(AWBC10,0x67);
    wrReg(AWBC11,0x49);
    wrReg(AWBC12,0x0e);
	wrReg(REG_GFIX,0x00);
	//wrReg(GGAIN,0);
	wrReg(AWBCTR3,0x0a);
	wrReg(AWBCTR2,0x55);
	wrReg(AWBCTR1,0x11);
	wrReg(AWBCTR0,0x9f);
	wrReg(0xb0,0x84);//Why does this improve the quality so much???
	wrReg(REG_COM16,COM16_AWBGAIN);//disable auto denoise and edge enhancment
	//wrReg(REG_COM16,0);
	wrReg(0x4C,0);//disable denoise
	wrReg(0x76,0);//disable denoise
	wrReg(0x77,0);//disable denoise
	wrReg(0x7B,4);//brighten up shadows a bit end point 4
	wrReg(0x7C,8);//brighten up shadows a bit end point 8
	//wrReg(0x88,238);//darken highligts end point 176
	//wrReg(0x89,211);//try to get more highlight detail
	//wrReg(0x7A,60);//slope
	//wrReg(0x26,0xB4);//lower maxium stable operating range for AEC
	//hueSatMatrix(0,100);
	//ov7670_store_cmatrix();
	//wrReg(0x20,12);//set ADC range to 1.5x*/
	wrReg(REG_COM9,0x6A);//max gain to 128x
	wrReg(0x74,16);//disable digital gain
	//wrReg(0x93,15);//dummy line MSB*/
	wrReg(0x1e,33);//hflip
	wrReg(0x11,2);//divider
	//wrReg(0x2a,5);//href delay
#endif
}
void main(void)
{
	ADCSRA=(1<<ADEN)|(1<<ADPS2);//enable ADC
	DDRL|=8;
	ASSR &= ~(_BV(EXCLK) | _BV(AS2));
	//generate 8mhz clock
	TCCR5A =67;
	TCCR5B=17;
	OCR5A = 0;
	DDRC=0;
	//DDRE|=48;
	uint8_t error;
	tft_init();  //init TFT library
	tft_setDisplayDirect(DOWN2UP);
	#ifdef MT9D111
		tft_drawStringP(PSTR("MT9D111"),20,240,4,BLUE);
	#else
		tft_drawStringP(PSTR("OV7670"),20,240,4,BLUE);
	#endif
	tft_drawStringP(PSTR("Test"),60,220,4,RED);
	cli();
	TWSR&=~3;//disable prescaler for TWI
	TWBR=72;//set to 100khz
	initCam();
	tft_drawStringP(PSTR("Starting"),120,300,4,WHITE);
	menu();
}

#ifdef MT9D111
void redrawT(uint8_t z,uint16_t regD,uint8_t micro,uint8_t id,uint8_t bit8)
#else
void redrawT(uint8_t z,uint8_t regD)
#endif
{
	tft_fillRectangle(0,320,112,320,BLACK);
	#ifdef MT9D111
	if(micro)
		tft_drawStringP(PSTR("Reg,id:"),0,320,2,WHITE);
	else
	#endif
	tft_drawStringP(PSTR("Register:"),0,320,2,WHITE);
	utoa(z,(char *)buf,16);
	tft_drawString((char *)buf,0,176,2,WHITE);
	utoa(regD,(char *)buf,16);
	tft_drawString((char *)buf,16,320,2,WHITE);
	utoa(regD,(char *)buf,10);
	tft_drawString((char *)buf,32,320,2,WHITE);
	#ifdef MT9D111
		utoa(regD>>8,(char *)buf,2);
		tft_drawString((char *)buf,48,leadingZeros(48),4,WHITE);
		utoa(regD&255,(char *)buf,2);
		tft_drawString((char *)buf,80,leadingZeros(80),4,WHITE);  
	#else
		utoa(regD,(char *)buf,2);
		tft_drawString((char *)buf,80,leadingZeros(80),4,WHITE);
	#endif
}
#ifdef MT9D111
void editRegs(uint8_t microedit)
#else
void editRegs(void)
#endif
{
	uint16_t x,y,z;
	#ifdef MT9D111
		uint16_t val;
	#else
		uint8_t val;
	#endif
	uint8_t address=3;
	#ifdef MT9D111
		uint8_t bitm8=0;
		uint8_t maddr=0;
	#endif
	#ifdef MT9D111
		redrawGUI(microedit);
	#else
		redrawGUI();
	#endif
	#ifdef MT9D111
		if(microedit){
			wrReg16(0xC6,address|(1<<13)||(maddr<<8)|(bitm8<<15));
			val=rdReg16(0xC8);
		}else
			val=rdReg16(address);
	#else
		val=rdReg(address);
	#endif
  #ifdef MT9D111
  redrawT(address,val,microedit,maddr,bitm8);
  #else
  redrawT(address,val);
  #endif
	while (1){
    /*p = ts.getPoint();
    p.x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);*/
		getPoint(&x,&y,&z);
		if (z > 10){
			if (x >= 188 && x <= 256 && y <= 128){
				tft_setOrientation(1);
				do {
					capImg();
					//p = ts.getPoint();//wait for screen to be pressed
					getPoint(&x,&y,&z);
				} while (z < 10);
				tft_setDisplayDirect(DOWN2UP);
				#ifdef MT9D111
					redrawGUI(microedit);
				#else
					redrawGUI();
				#endif
				val=rdReg(address);
				redrawT(address,val);
			}
			else if (x >= 188 && x <= 212 && y >= 224)
				return;
			else if (x >= 188 && x <= 212 && y <= 232 && y >= 136){
				#ifdef MT9D111
					uint16_t stp=0;
				#else
					uint8_t stp=0;
				#endif
				do {
					wrReg(address,stp);
					_delay_ms(1000);//give register time to settle
					tft_setOrientation(1);
					capImg();
					utoa(stp,(char *)buf,10);
					tft_drawString((char *)buf,0,320,2,WHITE);
					//p = ts.getPoint();//wait for screen to be pressed
					getPoint(&x,&y,&z);
					stp++;
				} while (z < 10);
				tft_setDisplayDirect(DOWN2UP);
				#ifdef MT9D111
					redrawGUI(microedit);
				#else
					redrawGUI();
				#endif
				val=rdReg(address);
				redrawT(address,val);
			}
    else if (x >= 80 && x <= 112 && y > 64){
      uint8_t off=(y-64)/32;
      off=1<<off;
      val^=off;
      wrReg(address,val);
      val=rdReg(address);
      redrawT(address,val);
    }
    else if (x >= 126 && x <= 156){
     //change register value
     if (y <= 256 && y >= 224){
        val++;
        wrReg(address,val);
        val=rdReg(address);
        redrawT(address,val);
      }
      else if (y <= 224 && y >= 192){
        val--;
        wrReg(address,val);
        val=rdReg(address);
        redrawT(address,val);
      }
      else if (y <= 160 && y >= 136){
        val+=16;
        wrReg(address,val);
        val=rdReg(address);
        redrawT(address,val);
      }
      else if (y <= 112 && y >= 88){
        val-=16;
        wrReg(address,val);
        val=rdReg(address);
        redrawT(address,val);
      }
    }
    else if (x >= 156 && x <= 188){
      if (y <= 256 && y >= 224){
        ++address;
        val=rdReg(address);
        redrawT(address,val);
      }
      else if (y <= 224 && y >= 192){
        --address;
        val=rdReg(address);
        redrawT(address,val);
      }
    }
  }
  uint8_t val2=rdReg(address);
  if (val2 != val){
   val=val2;
   redrawT(address,val);
  }
  }//end of loop
   //tft_paintScreenBlack();
}
void menu(void)
{
	uint16_t x=0,y=0,z=0;
	uint8_t item=0;
againMenu:
	while (1){
		tft_paintScreenBlack();
		//tft_setDisplayDirect(DOWN2UP);
		tft_drawStringP(PSTR("Register edit"),0,320,3,WHITE);
		#ifdef MT9D111
			tft_drawStringP(PSTR("Reg edit micro"),24,320,3,WHITE);
		#else
			tft_drawStringP(PSTR("Use Matrix 1"),24,320,3,WHITE);
			tft_drawStringP(PSTR("Use Matrix 2"),48,320,3,WHITE);
			tft_drawStringP(PSTR("Use Matrix 3"),72,320,3,WHITE);
		#endif
		tft_drawStringP(PSTR("Reset Reg"),96,320,3,WHITE);
		tft_drawStringP(PSTR("Compare Matrix"),120,320,3,WHITE);
		do {
			//p = ts.getPoint();//wait for screen to be pressed
			getPoint(&x,&y,&z);
			sprintf_P((char *)buf,PSTR("x: %d y: %d z: %d"),x,y,z);
			tft_fillRectangle(210,320,24,240,BLACK);
			tft_drawString((char *)buf,210,320,1,WHITE);
		}while (z < 10);
		item=x/24;
		//_delay_ms(1000);
		//goto againMenu;
		switch (item) {
			case 0:
			#ifdef MT9D111
				editRegs(0);
			#else
				editRegs();
			#endif
		break;
		case 1:
			#ifdef MT9D111
				editRegs(1);
			#else
				setmatrix(0);
				tft_setOrientation(1);
				capImg();
				tft_setDisplayDirect(DOWN2UP);
			#endif
		break;
		#ifndef MT9D111
			case 2:
				setmatrix(1);
				tft_setOrientation(1);
				capImg();
				tft_setDisplayDirect(DOWN2UP);
			break;
			case 3:
				setmatrix(2);
				tft_setOrientation(1);
				capImg();
				tft_setDisplayDirect(DOWN2UP);
			break;
		#endif
		case 4:
			initCam();
		break;
		#ifndef MT9D111
   case 5:
   //compare matrixes
   do {
        //p = ts.getPoint();//wait for screen to be pressed
        getPoint(&x,&y,&z);
        uint8_t a;
		for (a=0;a<3;a++)
		{
			setmatrix(a);
			tft_setOrientation(1);
			capImg();
			tft_setDisplayDirect(DOWN2UP);
		}
	}while (z < 10);
	break;
		#endif
		}
	}
}
uint16_t leadingZeros(uint8_t x)
{
	uint8_t len=strlen((const char *)buf);
	uint16_t len2=320;
	len=8-len;
	while (len--){
		tft_drawChar('0',x, len2, 4, WHITE);
		len2-=32;
	}
	return len2;
}
static uint16_t editer;//current value
static uint16_t minE;
static uint16_t maxE;
void update_edit(const char * text)
{
	
}
