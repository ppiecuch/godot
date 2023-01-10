/*************************************************************************/
/*  latin.h                                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

// AUTO-GENERATED - don't edit and use 'conv.py' to re-generate.
// AUTO-GENERATED - Fri Mar 11 2022 12:27:20

#include "util.h"

// clang-format off
std::vector<FontPath> getLatin1(real_t x, real_t y) {
  const real_t tx = 140+x;
  const real_t ty = -390-78+y;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
  return {{
  /* d */ -1, /* v */ {{'m',-40+tx,350+ty,{
        { 'x', 0 },
        { 'y', 0 },
        { 'r', getR(-40+tx,350+ty,0+tx,60+350+ty) }}},{'l',0+tx,60+350+ty,{
        { 'x', 0 },
        { 'y', 0 },
        { 'f', 1 }}}}}};
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

std::vector<FontPath> getLatin2(real_t x, real_t y) {
  const real_t tx = 150+x;
  const real_t ty = -390-78+y;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
  return {{
  /* d */ -1, /* v */ {{'m',40+tx,350+ty,{
        { 'x', 0 },
        { 'y', 0 },
        { 'r', getR(40+tx,350+ty,0+tx,60+350+ty) }}},{'l',0+tx,60+350+ty,{
        { 'x', 0 },
        { 'y', 0 },
        { 'f', 1 }}}}}};
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

std::vector<FontPath> getLatin3(real_t x, real_t y) {
  const real_t tx = 77+x;
  const real_t ty = -30-78+y;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
  return {{
  /* d */ -1, /* v */ {{'m',0+tx,50+ty,{
        { 'r', getR(0+tx,50+ty,68-VERTEX_GAP2+tx,0+ty) },
        { 'y', 0 },
        { 'x', 0 }}},{'l',68-VERTEX_GAP2+tx,0+ty,{
        { 'r', getR(0+tx,50+ty,68-VERTEX_GAP2+tx,0+ty) },
        { 'y', 0 },
        { 'x', 0 },
        { 'f', 1 }}},{'l',68+VERTEX_GAP2+tx,0+ty,{
        { 'r', getR(68+VERTEX_GAP2+tx,0+ty,68+68+tx,50+ty) },
        { 'y', 0 },
        { 'x', 0 },
        { 'f', 1 },
        { 'v', 1 }}},{'l',68+68+tx,50+ty,{
        { 'y', 0 },
        { 'x', 0 },
        { 'f', 1 }}}}}};
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

std::vector<FontPath> getLatin4(real_t x, real_t y) {
  const real_t tx = 65+x;
  const real_t ty = -16-78+y;
  const real_t scale = 0.8;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
  return {{
  /* d */ 1, /* v */ {{'m',199.4*scale+tx,20.7*scale+ty,{
        { 'x', -1 },
        { 'y', -0.2 },
        { 'r', getCurveR(199.4*scale+tx,20.7*scale+ty,187.6*scale+tx,36.6*scale+ty,168.2*scale+tx,47.1*scale+ty,148.2*scale+tx,47.1*scale+ty,0) },
        { 'f', 1 }}},{'b',187.6*scale+tx,36.6*scale+ty,168.2*scale+tx,47.1*scale+ty,148.2*scale+tx,47.1*scale+ty,{
        { 'x', -1 },
        { 'y', -0.2 },
        { 'r', ROTATE_VERTICAL }}},{'b',129.1*scale+tx,47.1*scale+ty,112.1*scale+tx,36.6*scale+ty,95.3*scale+tx,25.5*scale+ty,{
        { 'x', -1 },
        { 'y', -0.2 }}},{'b',76.8*scale+tx,13.2*scale+ty,59.1*scale+tx,0*scale+ty,39.6*scale+tx,0*scale+ty,{
        { 'x', -1 },
        { 'y', -0.2 },
        { 'r', ROTATE_VERTICAL }}},{'b',22.3*scale+tx,0*scale+ty,10.9*scale+tx,8.9*scale+ty,0*scale+tx,20*scale+ty,{
        { 'x', -1 },
        { 'y', -0.2 }}}}}};
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

std::vector<FontPath> getLatin5(real_t x, real_t y) {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
  return {{
  /* d */ 1, /* v */ {{'a',145-50+x,-78+y,{
        { 'x', 0 },
        { 'y', 0 }}}}},{
  /* d */ 1, /* v */ {{'a',145+50+x,-78+y,{
        { 'x', 0 },
        { 'y', 0 }}}}}};
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

std::vector<FontPath> getLatin6(real_t x, real_t y) {
  const real_t tx = x;
  const real_t ty = y;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
  return {{
  /* d */ 1, /* v */ {{'m',112.7+tx,0.0+ty,{
        { 'r', getCurveR(112.7+tx,0.0+ty+tx,10.1+ty,110.1+tx,19.3+ty,105.0+tx,27.7+ty,0) },
        { 'x', 0 },
        { 'y', 0 },
        { 'f', 1 }}},{'b',112.7+tx,10.1+ty,110.1+tx,19.3+ty,105.0+tx,27.7+ty,{
        { 'x', 0 },
        { 'y', 0 }}},{'b',99.8+tx,36.1+ty,92.9+tx,42.8+ty,84.3+tx,47.7+ty,{
        { 'x', 0 },
        { 'y', 0 }}},{'b',75.7+tx,52.6+ty,66.7+tx,55.0+ty,57.3+tx,55.0+ty,{
        { 'x', 0 },
        { 'y', 0 }}},{'b',47.5+tx,55.0+ty,38.3+tx,52.6+ty,29.6+tx,47.7+ty,{
        { 'x', 0 },
        { 'y', 0 }}},{'b',20.8+tx,42.8+ty,13.8+tx,36.1+ty,8.5+tx,27.7+ty,{
        { 'x', 0 },
        { 'y', 0 }}},{'b',3.2+tx,19.3+ty,0.5+tx,10.1+ty,0.5+tx,0.0+ty,{
        { 'x', 0 },
        { 'y', 0 }}}}}};
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

std::vector<FontPath> getLatin7(real_t x, real_t y) {
  const real_t tx = 88+x;
  const real_t ty = -116+y;
  const real_t scale = 0.5;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
  return {{
  /* d */ 1, /* v */ {{'m',232*scale+tx,116*scale+ty,{
        { 'r', ROTATE_HORIZONTAL },
        { 'p', 1 },
        { 'f', 1 }}},{'b',232*scale+tx,180.1*scale+ty,180.1*scale+tx,232*scale+ty,116*scale+tx,232*scale+ty,{
        { 'r', ROTATE_VERTICAL }}},{'b',51.9*scale+tx,232*scale+ty,0*scale+tx,180.1*scale+ty,0*scale+tx,116*scale+ty,{
        { 'r', ROTATE_HORIZONTAL }}},{'b',0*scale+tx,51.9*scale+ty,51.9*scale+tx,0*scale+ty,116*scale+tx,0*scale+ty,{
        { 'r', ROTATE_VERTICAL }}},{'b',180.1*scale+tx,0*scale+ty,232*scale+tx,51.9*scale+ty,232*scale+tx,116*scale+ty,{
        { 'r', ROTATE_HORIZONTAL },
        { 'c', 1 }}}}}};
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

const std::vector<FontPath> DATA_LI = {{
  /* d */ 1, /* v */ {{'m',0,352,{
        { 'y', 0 },
        { 'f', 1 }}},{'l',0,130,{
        { 'y', -3 }}}}}};
const std::vector<FontPath> DATA_LJ = {{
  /* d */ 1, /* v */ {{'m',0-115.9,317+127,{
        { 'x', 0.4 },
        { 'y', 0.63 },
        { 'r', getCurveR(0-115.9,317+127,12.6-115.9,327.4+127,29.6-115.9,333.2+127,45.9-115.9,334.2+127,0) },
        { 'f', 1 }}},{'b',12.6-115.9,327.4+127,29.6-115.9,333.2+127,45.9-115.9,334.2+127,{
        { 'x', 0.4 },
        { 'y', 0.63 },
        { 'r', ROTATE_VERTICAL }}},{'b',84.5-115.9,336.5+127,115.9-115.9,308.1+127,115.9-115.9,269.4+127,{
        { 'x', 0.4 },
        { 'y', 0.63 },
        { 'r', ROTATE_HORIZONTAL }}},{'l',115.9-115.9,0+127+3,{
        { 'y', -3 }}}}}};
const std::map<char32_t, FontData> LATIN = {
  { u'À', generateFontData(620,290,352,0,0,0,0,concatPaths(DATA_UA, getLatin1(0, 20))) },
  { u'Á', generateFontData(620,290,352,0,0,0,0,concatPaths(DATA_UA, getLatin2(0, 20))) },
  { u'Â', generateFontData(620,290,352,0,0,0,0,concatPaths(DATA_UA, getLatin3(0, 0))) },
  { u'Ã', generateFontData(620,290,352,0,0,0,0,concatPaths(DATA_UA, getLatin4(0, 20))) },
  { u'Ä', generateFontData(620,290,352,0,0,0,0,concatPaths(DATA_UA, getLatin5(0, 20))) },
  { u'Å', generateFontData(620,290,352,0,0,0,0,concatPaths(DATA_UA, getLatin7(0, 0))) },
  { u'à', generateFontData(600,232,232,10,2,-64,-64,concatPaths(DATA_LA, getLatin1(-29, -15))) },
  { u'á', generateFontData(600,232,232,10,2,-64,-64,concatPaths(DATA_LA, getLatin2(-29, -15))) },
  { u'â', generateFontData(600,232,232,10,2,-64,-64,concatPaths(DATA_LA, getLatin3(-29, -15))) },
  { u'ã', generateFontData(600,232,232,10,2,-64,-64,concatPaths(DATA_LA, getLatin4(-29, -15))) },
  { u'ä', generateFontData(600,232,232,10,2,-64,-64,concatPaths(DATA_LA, getLatin5(9, -15))) },
  { u'å', generateFontData(600,232,232,10,2,-64,-64,concatPaths(DATA_LA, getLatin7(-30, 0))) },
  { u'È', generateFontData(520,192,352,-5,-80,0,0,concatPaths(DATA_UE, getLatin1(-40, 0))) },
  { u'É', generateFontData(520,192,352,-5,-80,0,0,concatPaths(DATA_UE, getLatin2(-40, 0))) },
  { u'Ê', generateFontData(520,192,352,-5,-80,0,0,concatPaths(DATA_UE, getLatin3(-49, 0))) },
  { u'Ë', generateFontData(520,192,352,-5,-80,0,0,concatPaths(DATA_UE, getLatin5(-40, 0))) },
  { u'è', generateFontData(570,225.5,233.1,0,0,-64,-64,concatPaths(DATA_LE, getLatin1(-28, -14))) },
  { u'é', generateFontData(570,225.5,233.1,0,0,-64,-64,concatPaths(DATA_LE, getLatin2(-28, -14))) },
  { u'ê', generateFontData(570,225.5,233.1,0,0,-64,-64,concatPaths(DATA_LE, getLatin3(-28, -14))) },
  { u'ë', generateFontData(570,225.5,233.1,0,0,-64,-64,concatPaths(DATA_LE, getLatin5(-28, -14))) },
  { u'Ì', generateFontData(249,0,352,0,0,0,0,concatPaths(DATA_UI, getLatin1(-145, 0))) },
  { u'Í', generateFontData(249,0,352,0,0,0,0,concatPaths(DATA_UI, getLatin2(-145, 0))) },
  { u'Î', generateFontData(249,0,352,0,0,0,0,concatPaths(DATA_UI, getLatin3(-145, 0))) },
  { u'Ï', generateFontData(249,0,352,0,0,0,0,concatPaths(DATA_UI, getLatin5(-145, 0))) },
  { u'ì', generateFontData(200,0,352,0,0,0,0,concatPaths(DATA_LI, getLatin1(-145, 109))) },
  { u'í', generateFontData(200,0,352,0,0,0,0,concatPaths(DATA_LI, getLatin2(-145, 109))) },
  { u'î', generateFontData(200,0,352,0,0,0,0,concatPaths(DATA_LI, getLatin3(-145, 109))) },
  { u'ï', generateFontData(200,0,352,0,0,0,0,concatPaths(DATA_LI, getLatin5(-145, 109))) },
  { u'Ñ', generateFontData(721,250,352,0,0,0,0,concatPaths(DATA_UN, getLatin4(-20, 0))) },
  { u'ñ', generateFontData(520,182,352,0,0,0,0,concatPaths(DATA_LN, getLatin4(-54, 109))) },
  { u'Ò', generateFontData(850,360,360,0,0,0,0,concatPaths(DATA_UO, getLatin1(35, 4))) },
  { u'Ó', generateFontData(850,360,360,0,0,0,0,concatPaths(DATA_UO, getLatin2(35, 4))) },
  { u'Ô', generateFontData(850,360,360,0,0,0,0,concatPaths(DATA_UO, getLatin3(35, 4))) },
  { u'Õ', generateFontData(850,360,360,0,0,0,0,concatPaths(DATA_UO, getLatin4(35, 4))) },
  { u'Ö', generateFontData(850,360,360,0,0,0,0,concatPaths(DATA_UO, getLatin5(35, 4))) },
  { u'Ø', generateFontData(850,360,360,0,0,0,0,{{
  /* d */ 1, /* v */ {{'m',0,360,{
        { 'r', getR(0,360,360,0) },
        { 'f', 1 },
        { 'x', 0 },
        { 'y', 1 }}},{'l',360,0,{
        { 'x', 0 },
        { 'y', 1 }}}}}}) },
  { u'ò', generateFontData(580,232,232,0,0,-64,-64,concatPaths(DATA_LO, getLatin1(-29, -15))) },
  { u'ó', generateFontData(580,232,232,0,0,-64,-64,concatPaths(DATA_LO, getLatin2(-29, -15))) },
  { u'ô', generateFontData(580,232,232,0,0,-64,-64,concatPaths(DATA_LO, getLatin3(-29, -15))) },
  { u'õ', generateFontData(580,232,232,0,0,-64,-64,concatPaths(DATA_LO, getLatin4(-29, -15))) },
  { u'ö', generateFontData(580,232,232,0,0,-64,-64,concatPaths(DATA_LO, getLatin5(-29, -15))) },
  { u'ø', generateFontData(580,232,232,0,0,-64,-64,{{
  /* d */ 1, /* v */ {{'m',0,232,{
        { 'r', getR(0,232,232,0) },
        { 'f', 1 },
        { 'x', 0 },
        { 'y', 1 }}},{'l',232,0,{
        { 'x', 0 },
        { 'y', 1 }}}}}}) },
  { u'Ù', generateFontData(712,250,355,0,0,-0.5,-0.5,concatPaths(DATA_UU, getLatin1(-20, 1))) },
  { u'Ú', generateFontData(712,250,355,0,0,-0.5,-0.5,concatPaths(DATA_UU, getLatin2(-20, 1))) },
  { u'Û', generateFontData(712,250,355,0,0,-0.5,-0.5,concatPaths(DATA_UU, getLatin3(-20, 1))) },
  { u'Ŭ', generateFontData(712,250,355,0,0,-0.5,-0.5,concatPaths(DATA_UU, getLatin6(69, -107))) },
  { u'Ü', generateFontData(712,250,355,0,0,-0.5,-0.5,concatPaths(DATA_UU, getLatin5(-20, 1))) },
  { u'ù', generateFontData(520,182,352,0,0,0,0,concatPaths(DATA_LU, getLatin1(-54, 109))) },
  { u'ú', generateFontData(520,182,352,0,0,0,0,concatPaths(DATA_LU, getLatin2(-54, 109))) },
  { u'û', generateFontData(520,182,352,0,0,0,0,concatPaths(DATA_LU, getLatin3(-54, 109))) },
  { u'ŭ', generateFontData(520,182,352,0,0,0,0,concatPaths(DATA_LU, getLatin6(34, 0))) },
  { u'ü', generateFontData(520,182,352,0,0,0,0,concatPaths(DATA_LU, getLatin5(-54, 109))) },
  { u'Ý', generateFontData(673,270,352,0,0,0,0,concatPaths(DATA_UY, getLatin2(-10, 0))) },
  { u'ý', generateFontData(500,225.5,331.5,10,10,-119,-119,concatPaths(DATA_LY, getLatin2(-30, -20))) },
  { u'ÿ', generateFontData(500,225.5,331.5,10,10,-119,-119,concatPaths(DATA_LY, getLatin5(-30, -20))) },
  { u'Ĉ', generateFontData(700,293.1,360,0,0,0,0,concatPaths(DATA_UC, getLatin3(20, 4))) },
  { u'Ĝ', generateFontData(840,352,360,0,0,0,0,concatPaths(DATA_UG, getLatin3(30, 4))) },
  { u'Ĥ', generateFontData(684,232,352,0,0,0,0,concatPaths(DATA_UH, getLatin3(-29, 0))) },
  { u'Ĵ', generateFontData(472,172.5,355.5,10,20,-2,-2,concatPaths(DATA_UJ, getLatin3(-50, 0))) },
  { u'Ŝ', generateFontData(560,224,360,0,0,0,0,concatPaths(DATA_US, getLatin3(-30, 4))) },
  { u'ĉ', generateFontData(520,212.1,233.1,2,-10,-64,-64,concatPaths(DATA_LC, getLatin3(-29, -14))) },
  { u'ĝ', generateFontData(600,232,338,10,2,-117,-117,concatPaths(DATA_LG, getLatin3(-29, -15))) },
  { u'ĥ', generateFontData(520,182,352,0,0,0,0,concatPaths(DATA_LH, getLatin3(-52, 9))) },
  { u'ĵ', generateFontData(220,115.9,352,-60,-60,0,0,concatPaths(DATA_LJ, getLatin3(-155, 109))) },
  { u'ŝ', generateFontData(400,224*0.642,360*0.642,0,0,-64,-64,concatPaths(DATA_LS, getLatin3(-73, -15))) },
};
// clang-format on
