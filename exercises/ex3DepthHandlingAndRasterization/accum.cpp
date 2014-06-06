
#define _USE_MATH_DEFINES
#include <cmath>

#include "accum.h"


void accFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearP, GLdouble farP, GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble eyedz,
		GLdouble focus) {
	GLdouble xwsize, ywsize;
	GLdouble dx, dy;
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);

	xwsize = right - left;
	ywsize = top - bottom;
	dx = -(pixdx * xwsize / (GLdouble) viewport[2] + eyedx * nearP / focus);
	dy = -(pixdy * ywsize / (GLdouble) viewport[3] + eyedy * nearP / focus);

	glFrustum(left + dx, right + dx, bottom + dy, top + dy, nearP, farP);
}

void accPerspective(GLdouble fovy, GLdouble aspect, GLdouble nearP, GLdouble farP, GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble eyedz, GLdouble focus) {
	GLdouble fov2, left, right, bottom, top;
	fov2 = ((fovy * M_PI) / 180.0) / 2.0;

	top = nearP / (cos(fov2) / sin(fov2));
	bottom = -top;
	right = top * aspect;
	left = -right;

	accFrustum(left, right, bottom, top, nearP, farP, pixdx, pixdy, eyedx, eyedy, eyedz, focus);
}

