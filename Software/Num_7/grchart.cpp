/**
 ** grchart.c ---- Draw charts using the MGRX library
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

#include <stdio.h>
#include <stdlib.h>
#include "mgrx.h"
#include "grchart.h"

typedef struct {
    int x, y;
    int w, h;
    GrContext *ctx;
} SubArea;

static void DrawGrid(SubArea * a, GrChartData * d, GrChartStyle * s);
static void DrawBars(SubArea * a, GrChartData * d, GrChartStyle * s);
static void DrawLines(SubArea * a, GrChartData * d, GrChartStyle * s);
static void DrawAreas(SubArea * a, GrChartData * d, GrChartStyle * s);
static void DrawAreaLines(SubArea * a, GrChartData * d, GrChartStyle * s);
static void DrawAcuBars(SubArea * a, GrChartData * d, GrChartStyle * s);
static void DrawAcuAreas(SubArea * a, GrChartData * d, GrChartStyle * s);
static void DrawSLabels(SubArea * a, GrChartData * d, GrChartStyle * s);
static void DrawXLabels(SubArea * a, GrChartData * d, GrChartStyle * s);
static void DrawYLabels(SubArea * a, GrChartData * d, GrChartStyle * s);
static int CalcV4Width(GrChartData * d, GrChartStyle * s);
static int YScale(double y, SubArea * a, GrChartData * d);

/*
** GrChartDraw - Draws a chart in a context
**
** Arguments:
**   ctx:     Context to draw
**   d:       Chart data
**   s:       Chart style
**
** Returns  0 on success
**         -1 on error
**
**       +-------------------------------------+
**       |                 v1                  |
**       +----+----+----------------------+----+
**       |    |    |                      |    |
**       |    |    |                      |    |
**       | v3 | v6 |         v7           | v4 |
**       |    |    |                      |    |
**       |    +----+----------------------+    |
**       |    |    |         v5           |    |
**       +----+----+----------------------+----+
**       |    |              v2                |
**       +----+--------------------------------+
*/

int GrChartDraw(GrContext * ctx, GrChartData * d, GrChartStyle * s)
{
    GrContext ctxaux;
    SubArea v0, v1, v2, v3, v4, v5, v6, v7;
    GrTextOption totitle, toxtitle, toytitle;

    GrSaveContext(&ctxaux);
    if (ctx == NULL)
	ctx = &ctxaux;
    GrSetContext(ctx);

    totitle.txo_font = s->ftitle;
    totitle.txo_fgcolor = s->ctitle;
    totitle.txo_bgcolor = GrNOCOLOR;
    totitle.txo_chrtype = GR_BYTE_TEXT;
    totitle.txo_direct = GR_TEXT_RIGHT;
    totitle.txo_xalign = GR_ALIGN_CENTER;
    totitle.txo_yalign = GR_ALIGN_CENTER;

    toxtitle.txo_font = s->fxytitle;
    toxtitle.txo_fgcolor = s->cxytitle;
    toxtitle.txo_bgcolor = GrNOCOLOR;
    toxtitle.txo_chrtype = GR_BYTE_TEXT;
    toxtitle.txo_direct = GR_TEXT_RIGHT;
    toxtitle.txo_xalign = GR_ALIGN_CENTER;
    toxtitle.txo_yalign = GR_ALIGN_CENTER;

    toytitle.txo_font = s->fxytitle;
    toytitle.txo_fgcolor = s->cxytitle;
    toytitle.txo_bgcolor = GrNOCOLOR;
    toytitle.txo_chrtype = GR_BYTE_TEXT;
    toytitle.txo_direct = GR_TEXT_UP;
    toytitle.txo_xalign = GR_ALIGN_CENTER;
    toytitle.txo_yalign = GR_ALIGN_CENTER;

    v0.x = 0;
    v0.y = 0;
    v0.w = GrSizeX();
    v0.h = GrSizeY();
    v0.ctx = ctx;

    /* Title sub-area */
    v1.x = v0.x;
    v1.y = v0.y;
    v1.w = v0.w;
    v1.h = 2;
    if (d->title)
	v1.h += GrStringHeight(d->title, 0, &totitle);
    v1.ctx =
	GrCreateSubContext(v1.x, v1.y, v1.x + v1.w - 1, v1.y + v1.h - 1,
			   v0.ctx, NULL);

    /* X-Title sub-area */
    v2.w = v0.w;
    v2.h = 2;
    if (d->xtitle)
        v2.h += GrStringHeight(d->xtitle, 0, &toxtitle);
    v2.x = v0.x;
    v2.y = v0.y + v0.h - v2.h;
    v2.ctx =
	GrCreateSubContext(v2.x, v2.y, v2.x + v2.w - 1, v2.y + v2.h - 1,
			   v0.ctx, NULL);

    /* Y-Title sub-area */
    v3.w = 2;
    if (d->ytitle)
        v3.w += GrStringWidth(d->ytitle, 0, &toytitle);
    v3.h = v0.h - v1.h - v2.h;
    v3.x = v0.x;
    v3.y = v0.y + v1.h;
    v3.ctx =
	GrCreateSubContext(v3.x, v3.y, v3.x + v3.w - 1, v3.y + v3.h - 1,
			   v0.ctx, NULL);

    /* slabels sub-area */
    v4.w = v3.w;
    if (d->slabels) v4.w = CalcV4Width(d, s);
    v4.h = v3.h;
    v4.x = v0.x + v0.w - v4.w;
    v4.y = v3.y;
    v4.ctx =
	GrCreateSubContext(v4.x, v4.y, v4.x + v4.w - 1, v4.y + v4.h - 1,
			   v0.ctx, NULL);

    /* Pre-calcs for v5 & v6 */
    v5.h = GrFontCharHeight(s->flabels, 'M') + 4;
    v6.w = GrFontCharWidth(s->flabels, 'M') * 4 + 4;

    /* xlabels sub-area */
    v5.w = v0.w - v3.w - v4.w - v6.w;
    v5.x = v0.x + v3.w + v6.w;
    v5.y = v0.y + v0.h - v2.h - v5.h;
    v5.ctx =
	GrCreateSubContext(v5.x, v5.y, v5.x + v5.w - 1, v5.y + v5.h - 1,
			   v0.ctx, NULL); v6.h = v3.h - v5.h;

    /* ylabelse sub-area */
    v6.x = v0.x + v3.w;
    v6.y = v3.y;
    v6.ctx =
	GrCreateSubContext(v6.x, v6.y, v6.x + v6.w - 1, v6.y + v6.h - 1,
			   v0.ctx, NULL);

    /* chart sub-area */
    v7.w = v5.w;
    v7.h = v6.h;
    v7.x = v5.x;
    v7.y = v6.y;
    v7.ctx =
	GrCreateSubContext(v7.x, v7.y, v7.x + v7.w - 1, v7.y + v7.h - 1,
			   v0.ctx, NULL);

    GrSetContext(v0.ctx);
    GrClearContext(s->cbg);

    if (d->title) {
      GrSetContext(v1.ctx);
      GrDrawString(d->title, 0, v1.w / 2, v1.h / 2, &totitle);
    }

    if (d->xtitle) {
      GrSetContext(v2.ctx);
      GrDrawString(d->xtitle, 0, v2.w / 2, v2.h / 2, &toxtitle);
    }

    if (d->ytitle) {
      GrSetContext(v3.ctx);
      GrDrawString(d->ytitle, 0, v3.w / 2, v3.h / 2, &toytitle);
    }

    if (d->slabels) {
      GrSetContext(v4.ctx);
      DrawSLabels(&v4, d, s);
    }

    if (d->xlabels) {
      GrSetContext(v5.ctx);
      DrawXLabels(&v5, d, s);
    }

    GrSetContext(v6.ctx);
    DrawYLabels(&v6, d, s);

    GrSetContext(v7.ctx);
    GrClearContext(s->ccbg);
    DrawGrid(&v7, d, s);
    if (s->type == GR_CHART_TYPE_BARS)
	DrawBars(&v7, d, s);
    if (s->type == GR_CHART_TYPE_LINES)
	DrawLines(&v7, d, s);
    if (s->type == GR_CHART_TYPE_AREAS)
	DrawAreas(&v7, d, s);
    if (s->type == GR_CHART_TYPE_AREALINES)
        DrawAreaLines(&v7, d, s);
    if (s->type == GR_CHART_TYPE_ACUBARS)
        DrawAcuBars(&v7, d, s);
    if (s->type == GR_CHART_TYPE_ACUAREAS)
        DrawAcuAreas(&v7, d, s);

    GrSetContext(&ctxaux);
    GrDestroyContext(v7.ctx);
    GrDestroyContext(v6.ctx);
    GrDestroyContext(v5.ctx);
    GrDestroyContext(v4.ctx);
    GrDestroyContext(v3.ctx);
    GrDestroyContext(v2.ctx);
    GrDestroyContext(v1.ctx);
/*
    printf( "%d %d %d %d\n",v1.x,v1.y,v1.w,v1.h );
    printf( "%d %d %d %d\n",v2.x,v2.y,v2.w,v2.h );
    printf( "%d %d %d %d\n",v3.x,v3.y,v3.w,v3.h );
    printf( "%d %d %d %d\n",v4.x,v4.y,v4.w,v4.h );
    printf( "%d %d %d %d\n",v5.x,v5.y,v5.w,v5.h );
    printf( "%d %d %d %d\n",v6.x,v6.y,v6.w,v6.h );
    printf( "%d %d %d %d\n",v7.x,v7.y,v7.w,v7.h );
*/
    return 0;
}

static void DrawGrid(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    GrLineOption glo;
    double t;
    int y;
/*
    GrHLine(0, a->w - 1, a->h - 1, s->cfg);
    GrVLine(0, 0, a->h - 1, s->cfg);
    glo.lno_color = s->cfg;
    glo.lno_width = 1;
    glo.lno_pattlen = 2;
    glo.lno_dashpat = (unsigned char *)"\2\1";
    for (t = d->ymin; t <= d->ymax; t += d->ystep) {
	y = YScale(t, a, d);
	GrCustomLine(0, y, a->w - 1, y, &glo);
	*/
    for (t = d->ymin; t <= d->ymax; t += d->ystep) {
	y = YScale(t, a, d);
	GrLine(0, y, a->w - 1, y, GrAllocColor(210,210,210));
    }
}

static void DrawBars(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    int xstep, x, y, w, i, k;

    xstep = a->w / d->nsamples;
    for (i = 0; i < d->nsamples; i++) {
	x = i * xstep + 2;
	w = (xstep - 4) / d->nseries;
	for (k = 0; k < d->nseries; k++) {
	    y = YScale(d->yvalues[k * d->nseries + i], a, d);
	    GrFilledBox(x + w * k, y, x + w * (k + 1), a->h - 1,
			s->cvalues[k % 10]);
	    GrBox(x + w * k, y, x + w * (k + 1), a->h - 1, s->cfg);
	}
    }
}

static void DrawLines(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    int xstep, x1, x2, y1, y2, k, i;

    xstep = a->w / d->nsamples;
    for (k = 0; k < d->nseries; k++) {
	for (i = 0; i < d->nsamples - 1; i++) {
	    x1 = i * xstep + xstep / 2;
	    x2 = (i + 1) * xstep + xstep / 2;
            y1 = YScale(d->yvalues[k * d->nseries + i], a, d);
            y2 = YScale(d->yvalues[k * d->nseries + i + 1], a, d);
	    GrLine(x1, y1, x2, y2, s->cvalues[k % 10]);
	}
    }
}

static void DrawAreas(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    int xstep, x1, x2, y1, y2, k, i;
    int points[d->nsamples+2][2];

    xstep = a->w / d->nsamples;
    for (k = 0; k < d->nseries; k++) {
	for (i = 0; i < d->nsamples ; i++) {
            points[i][0] = i * xstep + xstep / 2;
            points[i][1] = YScale(d->yvalues[k * d->nseries + i], a, d);
	}
        points[d->nsamples][0] = points[d->nsamples-1][0];
        points[d->nsamples][1] = a->h - 1;
        points[d->nsamples+1][0] = points[0][0];
        points[d->nsamples+1][1] = a->h - 1;
	GrFilledPolygon(d->nsamples+2, points, s->cvalues[k % 10]);
	GrPolygon(d->nsamples+2, points, s->cfg);
    }
}

static void DrawAreaLines(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    int xstep, x1, x2, y1, y2, k, i;
    int points[d->nsamples+2][2];

    xstep = a->w / d->nsamples;
    k = 0;
    for (i = 0; i < d->nsamples ; i++) {
        points[i][0] = i * xstep + xstep / 2;
        points[i][1] = YScale(d->yvalues[k * d->nseries + i], a, d);
    }
    points[d->nsamples][0] = points[d->nsamples-1][0];
    points[d->nsamples][1] = a->h - 1;
    points[d->nsamples+1][0] = points[0][0];
    points[d->nsamples+1][1] = a->h - 1;
    GrFilledPolygon(d->nsamples+2, points, s->cvalues[k % 10]);
    GrPolygon(d->nsamples+2, points, s->cfg);

    for (k = 1; k < d->nseries; k++) {
	for (i = 0; i < d->nsamples - 1; i++) {
	    x1 = i * xstep + xstep / 2;
	    x2 = (i + 1) * xstep + xstep / 2;
            y1 = YScale(d->yvalues[k * d->nseries + i], a, d);
            y2 = YScale(d->yvalues[k * d->nseries + i + 1], a, d);
	    GrLine(x1, y1, x2, y2, s->cvalues[k % 10]);
	}
    }
}

static void DrawAcuBars(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    int xstep, x, y, w, i, k, yorg, ylast;

    xstep = a->w / d->nsamples;
    for (i = 0; i < d->nsamples; i++) {
	x = i * xstep + 2;
        w = xstep - 4;
        ylast = a->h -1;
	for (k = 0; k < d->nseries; k++) {
	    y = YScale(d->yvalues[k * d->nseries + i], a, d);
            yorg = y - (a->h - 1 - ylast);
            GrFilledBox(x, yorg, x + w, ylast, s->cvalues[k % 10]);
            GrBox(x, yorg, x + w, ylast, s->cfg);
            ylast = y;
	}
    }
}

static void DrawAcuAreas(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    int xstep, x1, x2, y1, y2, k, i, j;
    int points[d->nsamples+2][2];
    double yvalue;

    xstep = a->w / d->nsamples;
    for (k = d->nseries - 1; k >= 0; k--) {
	for (i = 0; i < d->nsamples ; i++) {
            yvalue = 0;
            for (j= k; j>=0; j--)
              yvalue += d->yvalues[j * d->nseries + i];
            points[i][0] = i * xstep + xstep / 2;
            points[i][1] = YScale(yvalue, a, d);
	}
        points[d->nsamples][0] = points[d->nsamples-1][0];
        points[d->nsamples][1] = a->h - 1;
        points[d->nsamples+1][0] = points[0][0];
        points[d->nsamples+1][1] = a->h - 1;
	GrFilledPolygon(d->nsamples+2, points, s->cvalues[k % 10]);
	GrPolygon(d->nsamples+2, points, s->cfg);
    }
}

static void DrawSLabels(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    int ystep, x, y, i;
    GrTextOption to;

    to.txo_font = s->flabels;
    to.txo_fgcolor = s->clabels;
    to.txo_bgcolor = GrNOCOLOR;
    to.txo_chrtype = GR_BYTE_TEXT;
    to.txo_direct = GR_TEXT_RIGHT;
    to.txo_xalign = GR_ALIGN_CENTER;
    to.txo_yalign = GR_ALIGN_BOTTOM;
    ystep = GrFontCharHeight(s->flabels, 'M') + 12;
    for (i = 0; i < d->nseries; i++) {
	x = a->w / 2;
        y = a->h - 1 - ystep * (i + 1);
        GrFilledBox( 2, y - ystep + 4, a->w - 3, y - ystep + 10,
		     s->cvalues[i % 10]);
        GrBox( 2, y - ystep + 4, a->w - 3, y - ystep + 10, s->cfg);
	GrDrawString(d->slabels[i], 0, x, y, &to);
    }
}

static void DrawXLabels(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    int xstep, x, y, i;
    GrTextOption to;

    to.txo_font = s->flabels;
    to.txo_fgcolor = s->clabels;
    to.txo_bgcolor = GrNOCOLOR;
    to.txo_chrtype = GR_BYTE_TEXT;
    to.txo_direct = GR_TEXT_RIGHT;
    to.txo_xalign = GR_ALIGN_CENTER;
    to.txo_yalign = GR_ALIGN_CENTER;
    xstep = a->w / d->nsamples;
    for (i = 0; i < d->nsamples; i++) {
	x = i * xstep + xstep / 2;
	GrVLine(x, 0, 3, s->cfg);
	GrDrawString(d->xlabels[i], 0, x, (a->h - 4) / 2 + 4, &to);
    }
}

static void DrawYLabels(SubArea * a, GrChartData * d, GrChartStyle * s)
{
    int y;
    double t;
    GrTextOption to;
    char saux[81];

    to.txo_font = s->flabels;
    to.txo_fgcolor = s->clabels;
    to.txo_bgcolor = GrNOCOLOR;
    to.txo_chrtype = GR_BYTE_TEXT;
    to.txo_direct = GR_TEXT_RIGHT;
    to.txo_xalign = GR_ALIGN_RIGHT;
    to.txo_yalign = GR_ALIGN_BOTTOM;
    for (t = d->ymin; t <= d->ymax; t += d->ylabelstep) {
	y = YScale(t, a, d);
	sprintf(saux, "%g", t);
	GrHLine(a->w - 3, a->w - 1, y, s->cfg);
	GrDrawString(saux, 0, a->w - 4, y, &to);
    }
}

static int CalcV4Width(GrChartData * d, GrChartStyle * s)
{
    int i, v, max = 0;

    for (i = 0; i < d->nseries; i++) {
        v = GrFontStringWidth(s->flabels, d->slabels[i], 0, GR_BYTE_TEXT);
        if (v > max) max = v;
    }

    return max + 4;
}

static int YScale(double y, SubArea * a, GrChartData * d)
{
    return ((a->h - 1) -
	    (((y - d->ymin) / (d->ymax - d->ymin)) * (a->h - 1)));
}

/*
** GrChartSetDefaultStyle - Initializes a Chart style with defaults
**
** Arguments:
**   s:       Chart style to be initialized
*/

void GrChartSetDefaultStyle(GrChartStyle * s)
{
    int i;

    s->ftitle = &GrFont_PC8x16;
    s->fxytitle = &GrFont_PC8x14;
    s->flabels = &GrFont_PC8x8;
    s->cbg = GrWhite();
    s->ccbg = GrWhite();
    s->cfg = GrBlack();
    for (i = 0; i < 10; i++)
	s->cvalues[i] = GrBlack();
    s->ctitle = GrBlack();
    s->cxytitle = GrBlack();
    s->clabels = GrBlack();
    s->type = GR_CHART_TYPE_BARS;
}
