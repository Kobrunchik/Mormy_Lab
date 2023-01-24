/**
 ** grchart.h ---- GrChart API
 **
 ** Copyright (C) 2002,2006 Mariano Alvarez Fernandez
 ** [e-mail: malfer at telefonica.net]
 **
 ** This library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Library General Public
 ** License as published by the Free Software Foundation; either
 ** version 2 of the License, or (at your option) any later version.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Library General Public License for more details.
 **
 ** You should have received a copy of the GNU Library General Public
 ** License along with this library; if not, write to the Free
 ** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 **
 **/

typedef struct {
    char *title;		/* main chart title (can be NULL) */
    char *xtitle;		/* X title (can be NULL) */
    char *ytitle;		/* Y title (can be NULL) */
    double ymin, ymax;		/* min and max Y values */
    double ystep;		/* grid Y step */
    double ylabelstep;		/* Y labels step */
    int nsamples;		/* number of samples by series */
    int nseries;		/* number of series */
    double *yvalues;		/* Y values for the nsamples * nseries */
    char **xlabels;		/* X labels for the nsamples (can be NULL) */
    char **slabels;		/* labels for the nseries (can be NULL) */
} GrChartData;

typedef struct {
    GrFont *ftitle;		/* main title font */
    GrFont *fxytitle;		/* font for X and Y titles */
    GrFont *flabels;		/* font for labels */
    GrColor cbg;		/* background color */
    GrColor ccbg;		/* background color for the graphic area */
    GrColor cfg;		/* foreground color */
    GrColor cvalues[10];	/* values colors for series 1 to 10 */
    GrColor ctitle;		/* main title color */
    GrColor cxytitle;		/* color for X and Y titles */
    GrColor clabels;		/* labels color */
    int type;			/* char type */
} GrChartStyle;

#define GR_CHART_TYPE_BARS       0
#define GR_CHART_TYPE_LINES      1
#define GR_CHART_TYPE_AREAS      2
#define GR_CHART_TYPE_AREALINES  3
#define GR_CHART_TYPE_ACUBARS    4
#define GR_CHART_TYPE_ACUAREAS   5

int GrChartDraw(GrContext * ctx, GrChartData * d, GrChartStyle * s);
void GrChartSetDefaultStyle(GrChartStyle * s);
