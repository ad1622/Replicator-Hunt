
#include "oogl/glIncludes.h"

void accFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearP, GLdouble farP, GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble eyedz,
		GLdouble focus);
void accPerspective(GLdouble fovy, GLdouble aspect, GLdouble nearP, GLdouble farP, GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble eyedz, GLdouble focus);
