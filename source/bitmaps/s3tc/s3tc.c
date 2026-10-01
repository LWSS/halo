/*
S3TC.C

*/

/* ---------- headers */

#include "cseries.h"
#include "s3tc.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct FCOLOR
{
	real rgba[4];
};

/* ---------- prototypes */

/* ---------- globals */

static real wtPrimary[3]= {0.082f, 0.6094f, 0.3086f};
static long mapRGB4[4]= {0, 2, 3, 1};
static long mapRGB3[4]= {0, 2, 1, 3};

/* ---------- private code */

static void ColorToFcolor(
	struct S3TC_COLOR *pcolor,
	struct FCOLOR *pfcolor)
{
	real value; // [fake name]
	long i;

	for (i= 0; i<3; i++)
	{
		value= pcolor->rgba[i];
		pfcolor->rgba[i]= value*wtPrimary[i]*(1.0f/255.0f);
	}

	return;
}

static void FcolorToColor(
	struct FCOLOR *pfcolor,
	struct S3TC_COLOR *pcolor)
{
	pcolor->rgba[0]= (byte)(pfcolor->rgba[0]/wtPrimary[0]*255.0f);
	pcolor->rgba[1]= (byte)(pfcolor->rgba[1]/wtPrimary[1]*255.0f);
	pcolor->rgba[2]= (byte)(pfcolor->rgba[2]/wtPrimary[2]*255.0f);

	return;
}

static void ColorToRGB(
	struct S3TC_COLOR *pcolor,
	word *prgb)
{
	word rgb= pcolor->rgba[2]>>3; // [fake name]

	rgb <<= 6;
	rgb |= pcolor->rgba[1]>>2;
	rgb <<= 5;
	rgb |= pcolor->rgba[0]>>3;
	*prgb= rgb;

	return;
}

static void RGBToColor(
	word const *prgb,
	struct S3TC_COLOR *pcolor)
{
	word rgb= *prgb; // [fake name]
	struct S3TC_COLOR color;

	color.rgba[0]= (byte)rgb;
	rgb >>= 5;
	color.rgba[1]= (byte)rgb;
	rgb >>= 6;
	color.rgba[2]= (byte)rgb;
	color.rgba[0] <<= 3;
	color.rgba[1] <<= 2;
	color.rgba[2] <<= 3;
	color.rgba[0] |= color.rgba[0]>>5;
	color.rgba[1] |= color.rgba[1]>>6;
	color.rgba[2] |= color.rgba[2]>>5;
	*pcolor= color;

	return;
}

static void Square3x3(
	real m[3][3],
	real m2[3][3])
{
	m2[0][0]= m[0][0]*m[0][0]+m[0][1]*m[0][1]+m[0][2]*m[0][2];
	m2[0][1]= (m[1][1]+m[0][0])*m[0][1]+m[1][2]*m[0][2];
	m2[0][2]= (m[2][2]+m[0][0])*m[0][2]+m[1][2]*m[0][1];
	m2[1][1]= m[0][1]*m[0][1]+m[1][1]*m[1][1]+m[1][2]*m[1][2];
	m2[1][2]= (m[2][2]+m[1][1])*m[1][2]+m[0][2]*m[0][1];
	m2[2][2]= m[0][2]*m[0][2]+m[1][2]*m[1][2]+m[2][2]*m[2][2];

	return;
}

static void Quantize(
	struct FCOLOR *pfcolor0,
	struct FCOLOR *pfcolor1,
	struct S3TCBlockRGB *pblock,
	long cOpaque)
{
	struct S3TC_COLOR color;
	word rgb; // [fake name]

	FcolorToColor(pfcolor0, &color);
	ColorToRGB(&color, &pblock->rgb0);
	FcolorToColor(pfcolor1, &color);
	ColorToRGB(&color, &pblock->rgb1);

	if ((cOpaque==16) ^ (pblock->rgb1<pblock->rgb0))
	{
		rgb= pblock->rgb0;
		pblock->rgb0= pblock->rgb1;
		pblock->rgb1= rgb;
	}

	RGBToColor(&pblock->rgb0, &color);
	ColorToFcolor(&color, pfcolor0);
	RGBToColor(&pblock->rgb1, &color);
	ColorToFcolor(&color, pfcolor1);

	return;
}

static void ClipExtrema(
	struct FCOLOR *plower,
	struct FCOLOR *pupper)
{
	struct FCOLOR *pmove; // [fake name]
	real t; // [fake name]
	long i;

	for (i= 0; i<3; i++)
	{
		if ((plower->rgba[i]<0.0f) ^ (pupper->rgba[i]<0.0f))
		{
			t= -(plower->rgba[i]/(pupper->rgba[i]-plower->rgba[i]));
			if (plower->rgba[i]<0.0f)
			{
				pmove= plower;
			}
			else
			{
				t -= 1.0f;
				pmove= pupper;
			}
			pmove->rgba[2] += t*(pupper->rgba[2]-plower->rgba[2]);
			pmove->rgba[1] += t*(pupper->rgba[1]-plower->rgba[1]);
			pmove->rgba[0] += t*(pupper->rgba[0]-plower->rgba[0]);
		}

		if ((plower->rgba[i]>wtPrimary[i]) ^ (pupper->rgba[i]>wtPrimary[i]))
		{
			t= (wtPrimary[i]-plower->rgba[i])/(pupper->rgba[i]-plower->rgba[i]);
			if (plower->rgba[i]>wtPrimary[i])
			{
				pmove= plower;
			}
			else
			{
				t -= 1.0f;
				pmove= pupper;
			}
			pmove->rgba[2] += t*(pupper->rgba[2]-plower->rgba[2]);
			pmove->rgba[1] += t*(pupper->rgba[1]-plower->rgba[1]);
			pmove->rgba[0] += t*(pupper->rgba[0]-plower->rgba[0]);
		}
	}

	return;
}

static void AllSame(
	struct S3TC_COLOR *pcolor,
	struct S3TCBlockRGB *pblock,
	word wAlpha)
{
	struct S3TC_COLOR color= *pcolor;
	word bit; // [fake name]
	unsigned long mask; // [fake name]
	long i;

	ColorToRGB(pcolor, &pblock->rgb0);
	pblock->rgb1= pblock->rgb0;
	pblock->pixbm= 0;

	if (wAlpha!=UNSIGNED_SHORT_MAX)
	{
		bit= 1;
		mask= 3;
		for (i= 0; i<16; i++)
		{
			if ((wAlpha & bit)==0)
			{
				pblock->pixbm |= mask;
			}
			else
			{
				color= pcolor[i];
			}
			bit <<= 1;
			mask <<= 2;
		}

		ColorToRGB(&color, &pblock->rgb0);
		pblock->rgb1= pblock->rgb0;
	}

	return;
}

/* ---------- public code */

void EncodeBlockRGB(
	struct S3TC_COLOR *colorSrc,
	struct S3TCBlockRGB *pblockDst)
{
	EncodeBlockRGBColorKey(colorSrc, pblockDst, 0);

	return;
}

void EncodeBlockRGBColorKey(
	struct S3TC_COLOR *colorSrc,
	struct S3TCBlockRGB *pblockDst,
	byte alphaKey)
{
	struct FCOLOR c[16];
	struct FCOLOR mean;
	struct FCOLOR fcolor0;
	struct FCOLOR fcolor1;
	struct FCOLOR axis_vector; // [fake name]
	real t[3][3];
	real t2[3][3]; // [fake name]
	real scale; // [fake name]
	real best; // [fake name]
	real inv; // [fake name]
	real len2; // [fake name]
	real min; // [fake name]
	real max; // [fake name]
	real projection; // [fake name]
	word wAlpha= 0;
	word bit; // [fake name]
	long cOpaque;
	long same; // [fake name]
	long axis; // [fake name]
	long i;
	long j;
	long k;

	if (pblockDst==NULL)
	{
		return;
	}

	cOpaque= 0;
	for (i= 15; i>=0; i--)
	{
		wAlpha <<= 1;
		if (colorSrc[i].rgba[3]<alphaKey)
		{
			wAlpha &= ~1;
		}
		else
		{
			wAlpha |= 1;
			cOpaque++;
		}
	}

	if (cOpaque==0)
	{
		pblockDst->rgb0= 0;
		pblockDst->rgb1= UNSIGNED_SHORT_MAX;
		pblockDst->pixbm= UNSIGNED_LONG_MAX;
		return;
	}

	same= TRUE;
	for (i= 0; i<16; i++)
	{
		if (same && i>0 &&
			(colorSrc[i].rgba[2]!=colorSrc[i-1].rgba[2] ||
			colorSrc[i].rgba[1]!=colorSrc[i-1].rgba[1] ||
			colorSrc[i].rgba[0]!=colorSrc[i-1].rgba[0]))
		{
			same= FALSE;
		}
	}
	if (same)
	{
		AllSame(colorSrc, pblockDst, wAlpha);
		return;
	}

	for (i= 0; i<16; i++)
	{
		ColorToFcolor(&colorSrc[i], &c[i]);
	}

	scale= 1.0f/cOpaque;
	for (j= 0; j<3; j++)
	{
		mean.rgba[j]= 0.0f;
		bit= 1;
		for (i= 0; i<16; i++)
		{
			if (wAlpha & bit)
			{
				mean.rgba[j] += c[i].rgba[j];
			}
			bit <<= 1;
		}
		mean.rgba[j] *= scale;
	}

	for (j= 0; j<3; j++)
	{
		for (i= 0; i<16; i++)
		{
			c[i].rgba[j] -= mean.rgba[j];
		}
	}

	for (i= 0; i<3; i++)
	{
		for (j= i; j<3; j++)
		{
			t[i][j]= 0.0f;
			bit= 1;
			for (k= 0; k<16; k++)
			{
				if (wAlpha & bit)
				{
					t[i][j] += c[k].rgba[j]*c[k].rgba[i];
				}
				bit <<= 1;
			}
		}
	}

	for (k= 0; k<9; k++)
	{
		real trace; // [fake name]

		Square3x3(t, t2);
		Square3x3(t2, t);
		trace= t[0][0]+t[1][1]+t[2][2];
		if (trace==0.0f)
		{
			AllSame(colorSrc, pblockDst, wAlpha);
			return;
		}
		scale= 3.0f/trace;
		for (i= 0; i<3; i++)
		{
			for (j= i; j<3; j++)
			{
				t[i][j] *= scale;
			}
		}
	}

	t[1][0]= t[0][1];
	t[2][0]= t[0][2];
	t[2][1]= t[1][2];

	best= 0.0f;
	if (t[0][0]>best)
	{
		best= t[0][0];
		axis= 0;
	}
	if (t[1][1]>best)
	{
		best= t[1][1];
		axis= 1;
	}
	if (t[2][2]>best)
	{
		best= t[2][2];
		axis= 2;
	}

	inv= 1.0f/(real)sqrt(best);
	len2= 0.0f;
	for (j= 0; j<3; j++)
	{
		axis_vector.rgba[j]= t[j][axis]*inv;
		len2 += axis_vector.rgba[j]*axis_vector.rgba[j];
	}
	if (len2==0.0f)
	{
		AllSame(colorSrc, pblockDst, wAlpha);
		return;
	}

	bit= 1;
	min= 99999.0f;
	max= -99999.0f;
	for (i= 0; i<16; i++)
	{
		if (wAlpha & bit)
		{
			projection= 0.0f;
			for (j= 0; j<3; j++)
			{
				projection +=axis_vector.rgba[j]*c[i].rgba[j];
			}
			projection /= len2;
			if (projection<min)
			{
				min= projection;
			}
			if (projection>max)
			{
				max= projection;
			}
		}
		bit <<= 1;
	}

	fcolor0.rgba[0]= axis_vector.rgba[0]*min+mean.rgba[0];
	fcolor1.rgba[0]= axis_vector.rgba[0]*max+mean.rgba[0];
	fcolor0.rgba[1]= axis_vector.rgba[1]*min+mean.rgba[1];
	fcolor1.rgba[1]= axis_vector.rgba[1]*max+mean.rgba[1];
	fcolor0.rgba[2]= axis_vector.rgba[2]*min+mean.rgba[2];
	fcolor1.rgba[2]= axis_vector.rgba[2]*max+mean.rgba[2];

	ClipExtrema(&fcolor0, &fcolor1);
	Quantize(&fcolor0, &fcolor1, pblockDst, cOpaque);

	len2= 0.0f;
	for (j= 0; j<3; j++)
	{
		len2 += (fcolor1.rgba[j]-fcolor0.rgba[j])*(fcolor1.rgba[j]-fcolor0.rgba[j]);
	}
	if (len2==0.0f && cOpaque==16)
	{
		AllSame(colorSrc, pblockDst, wAlpha);
		return;
	}

	bit= 0x8000;
	for (i= 15; i>=0; i--)
	{
		if (wAlpha & bit)
		{
			projection= 0.0f;
			for (j= 0; j<3; j++)
			{
				c[i].rgba[j] += mean.rgba[j];
				projection +=(c[i].rgba[j]-fcolor0.rgba[j])*(fcolor1.rgba[j]-fcolor0.rgba[j]);
			}
			projection /= len2;
			if (cOpaque==16)
			{
				projection *= 4.0f;
				if (projection<0.0f)
				{
					projection= 0.0f;
				}
				else if (projection>=4.0f)
				{
					projection= 3.0f;
				}
				pblockDst->pixbm <<= 2;
				pblockDst->pixbm |= mapRGB4[(long)projection];
			}
			else
			{
				projection *= 3.0f;
				if (projection<0.0f)
				{
					projection= 0.0f;
				}
				else if (projection>=3.0f)
				{
					projection= 2.0f;
				}
				pblockDst->pixbm <<= 2;
				pblockDst->pixbm |= mapRGB3[(long)projection];
			}
		}
		else
		{
			pblockDst->pixbm <<= 2;
			pblockDst->pixbm |= 3;
		}
		bit >>= 1;
	}

	return;
}

void DecodeBlockRGB(
	struct S3TCBlockRGB *pblockSrc,
	struct S3TC_COLOR *colorDst)
{
	struct S3TC_COLOR clut[4];
	unsigned long pixbm; // [fake name]
	long i;
	long j;

	if (pblockSrc==NULL)
	{
		memset(colorDst, 0, 16*sizeof(struct S3TC_COLOR));
	}
	else
	{
		RGBToColor(&pblockSrc->rgb0, &clut[0]);
		RGBToColor(&pblockSrc->rgb1, &clut[1]);
		clut[0].rgba[3]= clut[1].rgba[3]= clut[2].rgba[3]= 0xff;

		if (pblockSrc->rgb0>pblockSrc->rgb1)
		{
			word c0; // [fake name]
			word c1; // [fake name]

			c0= clut[0].rgba[0];
			c1= clut[1].rgba[0];
			clut[2].rgba[0]= (byte)((2*c0+c1+1)/3);
			clut[3].rgba[0]= (byte)((2*c1+c0+1)/3);
			c0= clut[0].rgba[1];
			c1= clut[1].rgba[1];
			clut[2].rgba[1]= (byte)((2*c0+c1+1)/3);
			clut[3].rgba[1]= (byte)((2*c1+c0+1)/3);
			c0= clut[0].rgba[2];
			c1= clut[1].rgba[2];
			clut[2].rgba[2]= (byte)((2*c0+c1+1)/3);
			clut[3].rgba[2]= (byte)((2*c1+c0+1)/3);
			clut[3].rgba[3]= 0xff;
		}
		else
		{
			for (j= 0; j<3; j++)
			{
				clut[2].rgba[j]= (byte)((clut[0].rgba[j]+clut[1].rgba[j])/2);
				clut[3].rgba[j]= 0;
			}
			clut[3].rgba[3]= 0;
		}

		pixbm= pblockSrc->pixbm;
		for (i= 0; i<16; i++)
		{
			for (j= 0; j<4; j++)
			{
				colorDst[i].rgba[j]= clut[pixbm & 3].rgba[j];
			}
			pixbm >>= 2;
		}
	}

	return;
}

void DecodeBlockRGB__single_pixel(
	struct S3TCBlockRGB const *pblockSrc,
	struct S3TC_COLOR *colorDst,
	short u,
	short v)
{
	struct S3TC_COLOR clut[4];
	long j;

	if (pblockSrc==NULL)
	{
		memset(colorDst, 0, 16*sizeof(struct S3TC_COLOR));
	}
	else
	{
		RGBToColor(&pblockSrc->rgb0, &clut[0]);
		RGBToColor(&pblockSrc->rgb1, &clut[1]);
		clut[0].rgba[3]= clut[1].rgba[3]= clut[2].rgba[3]= 0xff;

		if (pblockSrc->rgb0>pblockSrc->rgb1)
		{
			word c0; // [fake name]
			word c1; // [fake name]

			c0= clut[0].rgba[0];
			c1= clut[1].rgba[0];
			clut[2].rgba[0]= (byte)((2*c0+c1+1)/3);
			clut[3].rgba[0]= (byte)((2*c1+c0+1)/3);
			c0= clut[0].rgba[1];
			c1= clut[1].rgba[1];
			clut[2].rgba[1]= (byte)((2*c0+c1+1)/3);
			clut[3].rgba[1]= (byte)((2*c1+c0+1)/3);
			c0= clut[0].rgba[2];
			c1= clut[1].rgba[2];
			clut[2].rgba[2]= (byte)((2*c0+c1+1)/3);
			clut[3].rgba[2]= (byte)((2*c1+c0+1)/3);
			clut[3].rgba[3]= 0xff;
		}
		else
		{
			for (j= 0; j<3; j++)
			{
				clut[2].rgba[j]= (byte)((clut[0].rgba[j]+clut[1].rgba[j])/2);
				clut[3].rgba[j]= 0;
			}
			clut[3].rgba[3]= 0;
		}

		match_assert("c:\\halo\\SOURCE\\bitmaps\\s3tc\\s3tc.c", 773, u>=0 && u<=4);
		match_assert("c:\\halo\\SOURCE\\bitmaps\\s3tc\\s3tc.c", 774, v>=0 && v<=4);

		*colorDst= clut[(pblockSrc->pixbm>>(2*(4*v+u))) & 3];
	}

	return;
}

void EncodeBlockAlpha4(
	struct S3TC_COLOR *colorSrc,
	struct S3TCBlockAlpha4 *pblockDst)
{
	long i;
	long j;

	for (i= 0; i<4; i++)
	{
		for (j= 3; j>=0; j--)
		{
			pblockDst->alphabm[i] <<= 4;
			pblockDst->alphabm[i] |= colorSrc[4*i+j].rgba[3]>>4;
		}
	}

	EncodeBlockRGBColorKey(colorSrc, &pblockDst->rgb, 0);

	return;
}

void DecodeBlockAlpha4(
	struct S3TCBlockAlpha4 *pblockSrc,
	struct S3TC_COLOR *colorDst)
{
	long i;
	long j;
	word alpha; // [fake name]

	DecodeBlockRGB(&pblockSrc->rgb, colorDst);

	for (i= 0; i<4; i++)
	{
		alpha= pblockSrc->alphabm[i];

		for (j= 0; j<4; j++)
		{
			colorDst[4*i+j].rgba[3]= ((alpha & 0xf)<<4) | (alpha & 0xf);
			alpha >>= 4;
		}
	}

	return;
}

void DecodeBlockAlpha4__single_pixel(
	struct S3TCBlockAlpha4 const *pblockSrc,
	struct S3TC_COLOR *colorDst,
	short u,
	short v)
{
	word alpha; // [fake name]

	DecodeBlockRGB__single_pixel(&pblockSrc->rgb, colorDst, u, v);

	alpha= (pblockSrc->alphabm[v]>>(4*u)) & 0xf;
	colorDst->rgba[3]= (alpha<<4) | alpha;

	return;
}

void EncodeBlockAlpha3(
	struct S3TC_COLOR *colorSrc,
	struct S3TCBlockAlpha3 *pblockDst)
{
	byte alpha0; // [fake name]
	byte alpha1; // [fake name]
	byte alpha; // [fake name]
	long six_alpha; // [fake name]
	long range; // [fake name]
	long bias; // [fake name]
	long steps; // [fake name]
	long index; // [fake name]
	long i;
	unsigned long alphabm= 0; // [fake name]

	alpha0= alpha1= colorSrc[0].rgba[3];
	for (i= 1; i<16; i++)
	{
		alpha= colorSrc[i].rgba[3];
		if (alpha>alpha0)
		{
			alpha0= alpha;
		}
		if (alpha<alpha1)
		{
			alpha1= alpha;
		}
	}

	if (alpha0==255 && alpha1==0)
	{
		for (i= 0; i<16; i++)
		{
			alpha= colorSrc[i].rgba[3];
			if (alpha<alpha0 && alpha!=0)
			{
				alpha0= alpha;
			}
			if (alpha>alpha1 && alpha!=255)
			{
				alpha1= alpha;
			}
		}

		if (alpha0<alpha1)
		{
			six_alpha= TRUE;
		}
		else
		{
			alpha0= 255;
			alpha1= 0;
			six_alpha= FALSE;
		}
	}
	else
	{
		six_alpha= FALSE;
	}

	pblockDst->alpha0= alpha0;
	pblockDst->alpha1= alpha1;

	if (alpha0!=alpha1)
	{
		range= alpha0-alpha1;
		bias= range>>1;
		steps= six_alpha ? 5 : 7;

		for (i= 15; i>=0; i--)
		{
			alphabm <<= 3;
			if (six_alpha && colorSrc[i].rgba[3]==0)
			{
				alphabm |= 6;
			}
			else if (six_alpha && colorSrc[i].rgba[3]==255)
			{
				alphabm |= 7;
			}
			else
			{
				index= ((alpha0-colorSrc[i].rgba[3])*steps+bias)/range;
				if (index>=steps)
				{
					alphabm |= 1;
				}
				else if (index>0)
				{
					alphabm |= index+1;
				}
			}

			if ((i & 7)==0)
			{
				if (i==8)
				{
					pblockDst->alphabm[3]= (byte)alphabm;
					alphabm >>= 8;
					pblockDst->alphabm[4]= (byte)alphabm;
					alphabm >>= 8;
					pblockDst->alphabm[5]= (byte)alphabm;
				}
				else
				{
					pblockDst->alphabm[0]= (byte)alphabm;
					alphabm >>= 8;
					pblockDst->alphabm[1]= (byte)alphabm;
					alphabm >>= 8;
					pblockDst->alphabm[2]= (byte)alphabm;
				}
			}
		}
	}
	else
	{
		pblockDst->alphabm[0]= pblockDst->alphabm[1]= pblockDst->alphabm[2]=
			pblockDst->alphabm[3]= pblockDst->alphabm[4]= pblockDst->alphabm[5]= 0;
	}

	EncodeBlockRGBColorKey(colorSrc, &pblockDst->rgb, 0);

	return;
}

void DecodeBlockAlpha3(
	struct S3TCBlockAlpha3 *pblockSrc,
	struct S3TC_COLOR *colorDst)
{
	long i;
	long alpha[8];
	unsigned long alphabm= 0; // [fake name]

	DecodeBlockRGB(&pblockSrc->rgb, colorDst);

	alpha[0]= pblockSrc->alpha0;
	alpha[1]= pblockSrc->alpha1;

	if (alpha[0]>alpha[1])
	{
		alpha[2]= (6*alpha[0]+1*alpha[1])/7;
		alpha[3]= (5*alpha[0]+2*alpha[1])/7;
		alpha[4]= (4*alpha[0]+3*alpha[1])/7;
		alpha[5]= (3*alpha[0]+4*alpha[1])/7;
		alpha[6]= (2*alpha[0]+5*alpha[1])/7;
		alpha[7]= (1*alpha[0]+6*alpha[1])/7;
	}
	else
	{
		alpha[2]= (4*alpha[0]+1*alpha[1])/5;
		alpha[3]= (3*alpha[0]+2*alpha[1])/5;
		alpha[4]= (2*alpha[0]+3*alpha[1])/5;
		alpha[5]= (1*alpha[0]+4*alpha[1])/5;
		alpha[6]= 0;
		alpha[7]= 255;
	}

	for (i= 0; i<16; i++)
	{
		if ((i & 7)==0)
		{
			if (i==0)
			{
				alphabm= pblockSrc->alphabm[2];
				alphabm <<= 8;
				alphabm |= pblockSrc->alphabm[1];
				alphabm <<= 8;
				alphabm |= pblockSrc->alphabm[0];
			}
			else
			{
				alphabm= pblockSrc->alphabm[5];
				alphabm <<= 8;
				alphabm |= pblockSrc->alphabm[4];
				alphabm <<= 8;
				alphabm |= pblockSrc->alphabm[3];
			}
		}

		colorDst[i].rgba[3]= (byte)alpha[alphabm & 7];
		alphabm >>= 3;
	}

	return;
}

void DecodeBlockAlpha3__single_pixel(
	struct S3TCBlockAlpha3 const *pblockSrc,
	struct S3TC_COLOR *colorDst,
	short u,
	short v)
{
	word alut[8];
	unsigned long alphabm; // [fake name]

	DecodeBlockRGB__single_pixel(&pblockSrc->rgb, colorDst, u, v);

	alut[0]= pblockSrc->alpha0;
	alut[1]= pblockSrc->alpha1;

	if (alut[0]>alut[1])
	{
		alut[2]= (6*alut[0]+alut[1])/7;
		alut[3]= (5*alut[0]+2*alut[1])/7;
		alut[4]= (4*alut[0]+3*alut[1])/7;
		alut[5]= (3*alut[0]+4*alut[1])/7;
		alut[6]= (2*alut[0]+5*alut[1])/7;
		alut[7]= (alut[0]+6*alut[1])/7;
	}
	else
	{
		alut[2]= (4*alut[0]+alut[1])/5;
		alut[3]= (3*alut[0]+2*alut[1])/5;
		alut[4]= (2*alut[0]+3*alut[1])/5;
		alut[5]= (alut[0]+4*alut[1])/5;
		alut[6]= 0;
		alut[7]= 255;
	}

	if (v<2)
	{
		alphabm= pblockSrc->alphabm[2];
		alphabm <<= 8;
		alphabm |= pblockSrc->alphabm[1];
		alphabm <<= 8;
		alphabm |= pblockSrc->alphabm[0];
		alphabm >>= 3*(4*v+u);
		colorDst->rgba[3]= (byte)alut[alphabm & 7];
	}
	else
	{
		alphabm= pblockSrc->alphabm[5];
		alphabm <<= 8;
		alphabm |= pblockSrc->alphabm[4];
		alphabm <<= 8;
		alphabm |= pblockSrc->alphabm[3];
		alphabm >>= 3*(4*(v-2)+u);
		colorDst->rgba[3]= (byte)alut[alphabm & 7];
	}

	return;
}
