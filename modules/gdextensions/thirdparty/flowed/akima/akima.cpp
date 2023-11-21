#include "akima.h"
#include "fem.hpp" // Fortran EMulation library of fable module

namespace akima {

using namespace fem::major_types;

struct common_swpcom
{
  double swtol;

  common_swpcom() :
    swtol(fem::double0)
  {}
};

struct common_stcom
{
  double y;

  common_stcom() :
    y(fem::double0)
  {}
};

struct common :
  fem::common,
  common_swpcom,
  common_stcom
{
  fem::cmn_sve trfind_sve;
  fem::cmn_sve sdbi3p_sve;
  fem::cmn_sve sdsf3p_sve;
  fem::cmn_sve trlprt_sve;
  fem::cmn_sve trplot_sve;
  fem::cmn_sve trprnt_sve;

  common(
    int argc,
    char const* argv[])
  :
    fem::common(argc, argv)
  {}
};

void
trlist(
  int const& ncc,
  arr_cref<int> lcc,
  int const& n,
  arr_cref<int> list,
  arr_cref<int> lptr,
  arr_cref<int> lend,
  int const& nrow,
  int& nt,
  arr_ref<int, 2> ltri,
  arr_ref<int> lct,
  int& ier)
{
  lcc(dimension(star));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  ltri(dimension(nrow, star));
  lct(dimension(star));
  int nn = fem::int0;
  int lcc1 = fem::int0;
  int i = fem::int0;
  bool arcs = fem::bool0;
  int ka = fem::int0;
  int kt = fem::int0;
  int n1st = fem::int0;
  int nm2 = fem::int0;
  bool pass2 = fem::bool0;
  int j = fem::int0;
  int jlast = fem::int0;
  int n1 = fem::int0;
  int lpln1 = fem::int0;
  int lp2 = fem::int0;
  int n2 = fem::int0;
  int lp = fem::int0;
  int n3 = fem::int0;
  bool cstri = fem::bool0;
  int i1 = fem::int0;
  int i2 = fem::int0;
  int lpl = fem::int0;
  int kn = fem::int0;
  int i3 = fem::int0;
  int l = fem::int0;
  int isv = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   03/22/97
  //C
  //C   This subroutine converts a triangulation data structure
  //C from the linked list created by Subroutine TRMESH or
  //C TRMSHR to a triangle list.
  //C
  //C On input:
  //C
  //C       NCC = Number of constraints.  NCC .GE. 0.
  //C
  //C       LCC = List of constraint curve starting indexes (or
  //C             dummy array of length 1 if NCC = 0).  Refer to
  //C             Subroutine ADDCST.
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 3.
  //C
  //C       LIST,LPTR,LEND = Linked list data structure defin-
  //C                        ing the triangulation.  Refer to
  //C                        Subroutine TRMESH.
  //C
  //C       NROW = Number of rows (entries per triangle) re-
  //C              served for the triangle list LTRI.  The value
  //C              must be 6 if only the vertex indexes and
  //C              neighboring triangle indexes are to be
  //C              stored, or 9 if arc indexes are also to be
  //C              assigned and stored.  Refer to LTRI.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LTRI = Integer array of length at least NROW*NT,
  //C              where NT is at most 2N-5.  (A sufficient
  //C              length is 12N if NROW=6 or 18N if NROW=9.)
  //C
  //C       LCT = Integer array of length NCC or dummy array of
  //C             length 1 if NCC = 0.
  //C
  //C On output:
  //C
  //C       NT = Number of triangles in the triangulation unless
  //C            IER .NE. 0, in which case NT = 0.  NT = 2N - NB
  //C            - 2, where NB is the number of boundary nodes.
  //C
  //C       LTRI = NROW by NT array whose J-th column contains
  //C              the vertex nodal indexes (first three rows),
  //C              neighboring triangle indexes (second three
  //C              rows), and, if NROW = 9, arc indexes (last
  //C              three rows) associated with triangle J for
  //C              J = 1,...,NT.  The vertices are ordered
  //C              counterclockwise with the first vertex taken
  //C              to be the one with smallest index.  Thus,
  //C              LTRI(2,J) and LTRI(3,J) are larger than
  //C              LTRI(1,J) and index adjacent neighbors of
  //C              node LTRI(1,J).  For I = 1,2,3, LTRI(I+3,J)
  //C              and LTRI(I+6,J) index the triangle and arc,
  //C              respectively, which are opposite (not shared
  //C              by) node LTRI(I,J), with LTRI(I+3,J) = 0 if
  //C              LTRI(I+6,J) indexes a boundary arc.  Vertex
  //C              indexes range from 1 to N, triangle indexes
  //C              from 0 to NT, and, if included, arc indexes
  //C              from 1 to NA = NT+N-1.  The triangles are or-
  //C              dered on first (smallest) vertex indexes,
  //C              except that the sets of constraint triangles
  //C              (triangles contained in the closure of a con-
  //C              straint region) follow the non-constraint
  //C              triangles.
  //C
  //C       LCT = Array of length NCC containing the triangle
  //C             index of the first triangle of constraint J in
  //C             LCT(J).  Thus, the number of non-constraint
  //C             triangles is LCT(1)-1, and constraint J con-
  //C             tains LCT(J+1)-LCT(J) triangles, where
  //C             LCT(NCC+1) = NT+1.
  //C
  //C       IER = Error indicator.
  //C             IER = 0 if no errors were encountered.
  //C             IER = 1 if NCC, N, NROW, or an LCC entry is
  //C                     outside its valid range on input.
  //C             IER = 2 if the triangulation data structure
  //C                     (LIST,LPTR,LEND) is invalid.  Note,
  //C                     however, that these arrays are not
  //C                     completely tested for validity.
  //C
  //C Modules required by TRLIST:  None
  //C
  //C Intrinsic function called by TRLIST:  ABS
  //C
  //C***********************************************************
  //C
  //C Test for invalid input parameters and store the index
  //C   LCC1 of the first constraint node (if any).
  //C
  nn = n;
  if (ncc < 0 || (nrow != 6 && nrow != 9)) {
    goto statement_12;
  }
  lcc1 = nn + 1;
  if (ncc == 0) {
    if (nn < 3) {
      goto statement_12;
    }
  }
  else {
    FEM_DOSTEP(i, ncc, 1, -1) {
      if (lcc1 - lcc(i) < 3) {
        goto statement_12;
      }
      lcc1 = lcc(i);
    }
    if (lcc1 < 1) {
      goto statement_12;
    }
  }
  //C
  //C Initialize parameters for loop on triangles KT = (N1,N2,
  //C   N3), where N1 < N2 and N1 < N3.  This requires two
  //C   passes through the nodes with all non-constraint
  //C   triangles stored on the first pass, and the constraint
  //C   triangles stored on the second.
  //C
  //C   ARCS = TRUE iff arc indexes are to be stored.
  //C   KA,KT = Numbers of currently stored arcs and triangles.
  //C   N1ST = Starting index for the loop on nodes (N1ST = 1 on
  //C            pass 1, and N1ST = LCC1 on pass 2).
  //C   NM2 = Upper bound on candidates for N1.
  //C   PASS2 = TRUE iff constraint triangles are to be stored.
  //C
  arcs = nrow == 9;
  ka = 0;
  kt = 0;
  n1st = 1;
  nm2 = nn - 2;
  pass2 = false;
  //C
  //C Loop on nodes N1:  J = constraint containing N1,
  //C                    JLAST = last node in constraint J.
  //C
  statement_2:
  j = 0;
  jlast = lcc1 - 1;
  FEM_DO_SAFE(n1, n1st, nm2) {
    if (n1 > jlast) {
      //C
      //C N1 is the first node in constraint J+1.  Update J and
      //C   JLAST, and store the first constraint triangle index
      //C   if in pass 2.
      //C
      j++;
      if (j < ncc) {
        jlast = lcc(j + 1) - 1;
      }
      else {
        jlast = nn;
      }
      if (pass2) {
        lct(j) = kt + 1;
      }
    }
    //C
    //C Loop on pairs of adjacent neighbors (N2,N3).  LPLN1 points
    //C   to the last neighbor of N1, and LP2 points to N2.
    //C
    lpln1 = lend(n1);
    lp2 = lpln1;
    statement_3:
    lp2 = lptr(lp2);
    n2 = list(lp2);
    lp = lptr(lp2);
    n3 = fem::abs(list(lp));
    if (n2 < n1 || n3 < n1) {
      goto statement_10;
    }
    //C
    //C (N1,N2,N3) is a constraint triangle iff the three nodes
    //C   are in the same constraint and N2 < N3.  Bypass con-
    //C   straint triangles on pass 1 and non-constraint triangles
    //C   on pass 2.
    //C
    cstri = n1 >= lcc1 && n2 < n3 && n3 <= jlast;
    if ((cstri && !pass2) || (!cstri && pass2)) {
      goto statement_10;
    }
    //C
    //C Add a new triangle KT = (N1,N2,N3).
    //C
    kt++;
    ltri(1, kt) = n1;
    ltri(2, kt) = n2;
    ltri(3, kt) = n3;
    //C
    //C Loop on triangle sides (I1,I2) with neighboring triangles
    //C   KN = (I1,I2,I3).
    //C
    FEM_DO_SAFE(i, 1, 3) {
      if (i == 1) {
        i1 = n3;
        i2 = n2;
      }
      else if (i == 2) {
        i1 = n1;
        i2 = n3;
      }
      else {
        i1 = n2;
        i2 = n1;
      }
      //C
      //C Set I3 to the neighbor of I1 which follows I2 unless
      //C   I2->I1 is a boundary arc.
      //C
      lpl = lend(i1);
      lp = lptr(lpl);
      statement_4:
      if (list(lp) == i2) {
        goto statement_5;
      }
      lp = lptr(lp);
      if (lp != lpl) {
        goto statement_4;
      }
      //C
      //C   I2 is the last neighbor of I1 unless the data structure
      //C     is invalid.  Bypass the search for a neighboring
      //C     triangle if I2->I1 is a boundary arc.
      //C
      if (fem::abs(list(lp)) != i2) {
        goto statement_13;
      }
      kn = 0;
      if (list(lp) < 0) {
        goto statement_8;
      }
      //C
      //C   I2->I1 is not a boundary arc, and LP points to I2 as
      //C     a neighbor of I1.
      //C
      statement_5:
      lp = lptr(lp);
      i3 = fem::abs(list(lp));
      //C
      //C Find L such that LTRI(L,KN) = I3 (not used if KN > KT),
      //C   and permute the vertex indexes of KN so that I1 is
      //C   smallest.
      //C
      if (i1 < i2 && i1 < i3) {
        l = 3;
      }
      else if (i2 < i3) {
        l = 2;
        isv = i1;
        i1 = i2;
        i2 = i3;
        i3 = isv;
      }
      else {
        l = 1;
        isv = i1;
        i1 = i3;
        i3 = i2;
        i2 = isv;
      }
      //C
      //C Test for KN > KT (triangle index not yet assigned).
      //C
      if (i1 > n1 && !pass2) {
        goto statement_9;
      }
      //C
      //C Find KN, if it exists, by searching the triangle list in
      //C   reverse order.
      //C
      FEM_DOSTEP(kn, kt - 1, 1, -1) {
        if (ltri(1, kn) == i1 && ltri(2, kn) == i2 && ltri(3, kn) == i3) {
          goto statement_7;
        }
      }
      goto statement_9;
      //C
      //C Store KT as a neighbor of KN.
      //C
      statement_7:
      ltri(l + 3, kn) = kt;
      //C
      //C Store KN as a neighbor of KT, and add a new arc KA.
      //C
      statement_8:
      ltri(i + 3, kt) = kn;
      if (arcs) {
        ka++;
        ltri(i + 6, kt) = ka;
        if (kn != 0) {
          ltri(l + 6, kn) = ka;
        }
      }
      statement_9:;
    }
    //C
    //C Bottom of loop on triangles.
    //C
    statement_10:
    if (lp2 != lpln1) {
      goto statement_3;
    }
  }
  //C
  //C Bottom of loop on nodes.
  //C
  if (!pass2 && ncc > 0) {
    pass2 = true;
    n1st = lcc1;
    goto statement_2;
  }
  //C
  //C No errors encountered.
  //C
  nt = kt;
  ier = 0;
  return;
  //C
  //C Invalid input parameter.
  //C
  statement_12:
  nt = 0;
  ier = 1;
  return;
  //C
  //C Invalid triangulation data structure:  I1 is a neighbor of
  //C   I2, but I2 is not a neighbor of I1.
  //C
  statement_13:
  nt = 0;
  ier = 2;
}

void
insert(
  int const& k,
  int const& lp,
  arr_ref<int> list,
  arr_ref<int> lptr,
  int& lnew)
{
  list(dimension(star));
  lptr(dimension(star));
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   09/01/88
  //C
  //C   This subroutine inserts K as a neighbor of N1 following
  //C N2, where LP is the LIST pointer of N2 as a neighbor of
  //C N1.  Note that, if N2 is the last neighbor of N1, K will
  //C become the first neighbor (even if N1 is a boundary node).
  //C
  //C On input:
  //C
  //C       K = Index of the node to be inserted.
  //C
  //C       LP = LIST pointer of N2 as a neighbor of N1.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LIST,LPTR,LNEW = Data structure defining the trian-
  //C                        gulation.  Refer to Subroutine
  //C                        TRMESH.
  //C
  //C On output:
  //C
  //C       LIST,LPTR,LNEW = Data structure updated with the
  //C                        addition of node K.
  //C
  //C Modules required by INSERT:  None
  //C
  //C***********************************************************
  //C
  int lsav = lptr(lp);
  lptr(lp) = lnew;
  list(lnew) = k;
  lptr(lnew) = lsav;
  lnew++;
}

void
bdyadd(
  int const& kk,
  int const& i1,
  int const& i2,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& lnew)
{
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(star));
  int k = fem::int0;
  int n1 = fem::int0;
  int n2 = fem::int0;
  int lp = fem::int0;
  int lsav = fem::int0;
  int next = fem::int0;
  int nsav = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   02/22/91
  //C
  //C   This subroutine adds a boundary node to a triangulation
  //C of a set of points in the plane.  The data structure is
  //C updated with the insertion of node KK, but no optimization
  //C is performed.
  //C
  //C On input:
  //C
  //C       KK = Index of a node to be connected to the sequence
  //C            of all visible boundary nodes.  KK .GE. 1 and
  //C            KK must not be equal to I1 or I2.
  //C
  //C       I1 = First (rightmost as viewed from KK) boundary
  //C            node in the triangulation which is visible from
  //C            node KK (the line segment KK-I1 intersects no
  //C            arcs.
  //C
  //C       I2 = Last (leftmost) boundary node which is visible
  //C            from node KK.  I1 and I2 may be determined by
  //C            Subroutine TRFIND.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LIST,LPTR,LEND,LNEW = Triangulation data structure
  //C                             created by TRMESH or TRMSHR.
  //C                             Nodes I1 and I2 must be in-
  //C                             cluded in the triangulation.
  //C
  //C On output:
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure updated with
  //C                             the addition of node KK.  Node
  //C                             KK is connected to I1, I2, and
  //C                             all boundary nodes in between.
  //C
  //C Module required by BDYADD:  INSERT
  //C
  //C***********************************************************
  //C
  k = kk;
  n1 = i1;
  n2 = i2;
  //C
  //C Add K as the last neighbor of N1.
  //C
  lp = lend(n1);
  lsav = lptr(lp);
  lptr(lp) = lnew;
  list(lnew) = -k;
  lptr(lnew) = lsav;
  lend(n1) = lnew;
  lnew++;
  next = -list(lp);
  list(lp) = next;
  nsav = next;
  //C
  //C Loop on the remaining boundary nodes between N1 and N2,
  //C   adding K as the first neighbor.
  //C
  statement_1:
  lp = lend(next);
  insert(k, lp, list, lptr, lnew);
  if (next == n2) {
    goto statement_2;
  }
  next = -list(lp);
  list(lp) = next;
  goto statement_1;
  //C
  //C Add the boundary nodes between N1 and N2 as neighbors
  //C   of node K.
  //C
  statement_2:
  lsav = lnew;
  list(lnew) = n1;
  lptr(lnew) = lnew + 1;
  lnew++;
  next = nsav;
  //C
  statement_3:
  if (next == n2) {
    goto statement_4;
  }
  list(lnew) = next;
  lptr(lnew) = lnew + 1;
  lnew++;
  lp = lend(next);
  next = list(lp);
  goto statement_3;
  //C
  statement_4:
  list(lnew) = -n2;
  lptr(lnew) = lsav;
  lend(k) = lnew;
  lnew++;
}

bool
crtri(
  int const& ncc,
  arr_cref<int> lcc,
  int const& i1,
  int const& i2,
  int const& i3)
{
  bool return_value = fem::bool0;
  lcc(dimension(star));
  int imax = fem::int0;
  int i = fem::int0;
  int imin = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   08/14/91
  //C
  //C   This function returns TRUE if and only if triangle (I1,
  //C I2,I3) lies in a constraint region.
  //C
  //C On input:
  //C
  //C       NCC,LCC = Constraint data structure.  Refer to Sub-
  //C                 routine ADDCST.
  //C
  //C       I1,I2,I3 = Nodal indexes of the counterclockwise-
  //C                  ordered vertices of a triangle.
  //C
  //C Input parameters are altered by this function.
  //C
  //C       CRTRI = TRUE iff (I1,I2,I3) is a constraint region
  //C               triangle.
  //C
  //C Note that input parameters are not tested for validity.
  //C
  //C Modules required by CRTRI:  None
  //C
  //C Intrinsic functions called by CRTRI:  MAX, MIN
  //C
  //C***********************************************************
  //C
  imax = fem::max(i1, i2, i3);
  //C
  //C   Find the index I of the constraint containing IMAX.
  //C
  i = ncc + 1;
  statement_1:
  i = i - 1;
  if (i <= 0) {
    goto statement_2;
  }
  if (imax < lcc(i)) {
    goto statement_1;
  }
  imin = fem::min(i1, i2, i3);
  //C
  //C P lies in a constraint region iff I1, I2, and I3 are nodes
  //C   of the same constraint (IMIN >= LCC(I)), and (IMIN,IMAX)
  //C   is (I1,I3), (I2,I1), or (I3,I2).
  //C
  return_value = imin >= lcc(i) && ((imin == i1 && imax == i3) || (
    imin == i2 && imax == i1) || (imin == i3 && imax == i2));
  return return_value;
  //C
  //C NCC .LE. 0 or all vertices are non-constraint nodes.
  //C
  statement_2:
  return_value = false;
  return return_value;
}

int
indxcc(
  int const& ncc,
  arr_cref<int> lcc,
  int const& n,
  arr_cref<int> list,
  arr_cref<int> lend)
{
  int return_value = fem::int0;
  lcc(dimension(star));
  list(dimension(star));
  lend(dimension(n));
  int n0 = fem::int0;
  int lp = fem::int0;
  int i = fem::int0;
  int ilast = fem::int0;
  int ifrst = fem::int0;
  int nst = fem::int0;
  int nxt = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   08/25/91
  //C
  //C   Given a constrained Delaunay triangulation, this func-
  //C tion returns the index, if any, of an exterior constraint
  //C curve (an unbounded constraint region).  An exterior con-
  //C straint curve is assumed to be present if and only if the
  //C clockwise-ordered sequence of boundary nodes is a subse-
  //C quence of a constraint node sequence.  The triangulation
  //C adjacencies corresponding to constraint edges may or may
  //C not have been forced by a call to ADDCST, and the con-
  //C straint region may or may not be valid (contain no nodes).
  //C
  //C On input:
  //C
  //C       NCC = Number of constraints.  NCC .GE. 0.
  //C
  //C       LCC = List of constraint curve starting indexes (or
  //C             dummy array of length 1 if NCC = 0).  Refer to
  //C             Subroutine ADDCST.
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 3.
  //C
  //C       LIST,LEND = Data structure defining the triangula-
  //C                   tion.  Refer to Subroutine TRMESH.
  //C
  //C   Input parameters are not altered by this function.  Note
  //C that the parameters are not tested for validity.
  //C
  //C On output:
  //C
  //C       INDXCC = Index of the exterior constraint curve, if
  //C                present, or 0 otherwise.
  //C
  //C Modules required by INDXCC:  None
  //C
  //C***********************************************************
  //C
  return_value = 0;
  if (ncc < 1) {
    return return_value;
  }
  //C
  //C Set N0 to the boundary node with smallest index.
  //C
  n0 = 0;
  statement_1:
  n0++;
  lp = lend(n0);
  if (list(lp) > 0) {
    goto statement_1;
  }
  //C
  //C Search in reverse order for the constraint I, if any, that
  //C   contains N0.  IFRST and ILAST index the first and last
  //C   nodes in constraint I.
  //C
  i = ncc;
  ilast = n;
  statement_2:
  ifrst = lcc(i);
  if (n0 >= ifrst) {
    goto statement_3;
  }
  if (i == 1) {
    return return_value;
  }
  i = i - 1;
  ilast = ifrst - 1;
  goto statement_2;
  //C
  //C N0 is in constraint I which indexes an exterior constraint
  //C   curve iff the clockwise-ordered sequence of boundary
  //C   node indexes beginning with N0 is increasing and bounded
  //C   above by ILAST.
  //C
  statement_3:
  nst = n0;
  //C
  statement_4:
  nxt = -list(lp);
  if (nxt == nst) {
    goto statement_5;
  }
  if (nxt <= n0 || nxt > ilast) {
    return return_value;
  }
  n0 = nxt;
  lp = lend(n0);
  goto statement_4;
  //C
  //C Constraint I contains the boundary node sequence as a
  //C   subset.
  //C
  statement_5:
  return_value = i;
  return return_value;
}

int
lstptr(
  int const& lpl,
  int const& nb,
  arr_cref<int> list,
  arr_cref<int> lptr)
{
  int return_value = fem::int0;
  list(dimension(star));
  lptr(dimension(star));
  int lp = fem::int0;
  int nd = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   09/01/88
  //C
  //C   This function returns the index (LIST pointer) of NB in
  //C the adjacency list for N0, where LPL = LEND(N0).
  //C
  //C On input:
  //C
  //C       LPL = LEND(N0)
  //C
  //C       NB = Index of the node whose pointer is to be re-
  //C            turned.  NB must be connected to N0.
  //C
  //C       LIST,LPTR = Data structure defining the triangula-
  //C                   tion.  Refer to Subroutine TRMESH.
  //C
  //C Input parameters are not altered by this function.
  //C
  //C On output:
  //C
  //C       LSTPTR = Pointer such that LIST(LSTPTR) = NB or
  //C                LIST(LSTPTR) = -NB, unless NB is not a
  //C                neighbor of N0, in which case LSTPTR = LPL.
  //C
  //C Modules required by LSTPTR:  None
  //C
  //C***********************************************************
  //C
  lp = lptr(lpl);
  statement_1:
  nd = list(lp);
  if (nd == nb) {
    goto statement_2;
  }
  lp = lptr(lp);
  if (lp != lpl) {
    goto statement_1;
  }
  //C
  statement_2:
  return_value = lp;
  return return_value;
}

void
intadd(
  int const& kk,
  int const& i1,
  int const& i2,
  int const& i3,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& lnew)
{
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(star));
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   02/22/91
  //C
  //C   This subroutine adds an interior node to a triangulation
  //C of a set of points in the plane.  The data structure is
  //C updated with the insertion of node KK into the triangle
  //C whose vertices are I1, I2, and I3.  No optimization of the
  //C triangulation is performed.
  //C
  //C On input:
  //C
  //C       KK = Index of the node to be inserted.  KK .GE. 1
  //C            and KK must not be equal to I1, I2, or I3.
  //C
  //C       I1,I2,I3 = Indexes of the counterclockwise-ordered
  //C                  sequence of vertices of a triangle which
  //C                  contains node KK.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure defining the
  //C                             triangulation.  Refer to Sub-
  //C                             routine TRMESH.  Triangle
  //C                             (I1,I2,I3) must be included
  //C                             in the triangulation.
  //C
  //C On output:
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure updated with
  //C                             the addition of node KK.  KK
  //C                             will be connected to nodes I1,
  //C                             I2, and I3.
  //C
  //C Modules required by INTADD:  INSERT, LSTPTR
  //C
  //C***********************************************************
  //C
  int k = kk;
  //C
  //C Initialization.
  //C
  int n1 = i1;
  int n2 = i2;
  int n3 = i3;
  //C
  //C Add K as a neighbor of I1, I2, and I3.
  //C
  int lp = lstptr(lend(n1), n2, list, lptr);
  insert(k, lp, list, lptr, lnew);
  lp = lstptr(lend(n2), n3, list, lptr);
  insert(k, lp, list, lptr, lnew);
  lp = lstptr(lend(n3), n1, list, lptr);
  insert(k, lp, list, lptr, lnew);
  //C
  //C Add I1, I2, and I3 as neighbors of K.
  //C
  list(lnew) = n1;
  list(lnew + 1) = n2;
  list(lnew + 2) = n3;
  lptr(lnew) = lnew + 1;
  lptr(lnew + 1) = lnew + 2;
  lptr(lnew + 2) = lnew;
  lend(k) = lnew + 2;
  lnew += 3;
}

void
swap(
  int const& in1,
  int const& in2,
  int const& io1,
  int const& io2,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& lp21)
{
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(star));
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   06/22/98
  //C
  //C   Given a triangulation of a set of points on the unit
  //C sphere, this subroutine replaces a diagonal arc in a
  //C strictly convex quadrilateral (defined by a pair of adja-
  //C cent triangles) with the other diagonal.  Equivalently, a
  //C pair of adjacent triangles is replaced by another pair
  //C having the same union.
  //C
  //C On input:
  //C
  //C       IN1,IN2,IO1,IO2 = Nodal indexes of the vertices of
  //C                         the quadrilateral.  IO1-IO2 is re-
  //C                         placed by IN1-IN2.  (IO1,IO2,IN1)
  //C                         and (IO2,IO1,IN2) must be trian-
  //C                         gles on input.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LIST,LPTR,LEND = Data structure defining the trian-
  //C                        gulation.  Refer to Subroutine
  //C                        TRMESH.
  //C
  //C On output:
  //C
  //C       LIST,LPTR,LEND = Data structure updated with the
  //C                        swap -- triangles (IO1,IO2,IN1) and
  //C                        (IO2,IO1,IN2) are replaced by
  //C                        (IN1,IN2,IO2) and (IN2,IN1,IO1)
  //C                        unless LP21 = 0.
  //C
  //C       LP21 = Index of IN1 as a neighbor of IN2 after the
  //C              swap is performed unless IN1 and IN2 are
  //C              adjacent on input, in which case LP21 = 0.
  //C
  //C Module required by SWAP:  LSTPTR
  //C
  //C Intrinsic function called by SWAP:  ABS
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C LP,LPH,LPSAV = LIST pointers
  //C
  //C Test for IN1 and IN2 adjacent.
  //C
  int lp = lstptr(lend(in1), in2, list, lptr);
  if (fem::abs(list(lp)) == in2) {
    lp21 = 0;
    return;
  }
  //C
  //C Delete IO2 as a neighbor of IO1.
  //C
  lp = lstptr(lend(io1), in2, list, lptr);
  int lph = lptr(lp);
  lptr(lp) = lptr(lph);
  //C
  //C If IO2 is the last neighbor of IO1, make IN2 the
  //C   last neighbor.
  //C
  if (lend(io1) == lph) {
    lend(io1) = lp;
  }
  //C
  //C Insert IN2 as a neighbor of IN1 following IO1
  //C   using the hole created above.
  //C
  lp = lstptr(lend(in1), io1, list, lptr);
  int lpsav = lptr(lp);
  lptr(lp) = lph;
  list(lph) = in2;
  lptr(lph) = lpsav;
  //C
  //C Delete IO1 as a neighbor of IO2.
  //C
  lp = lstptr(lend(io2), in1, list, lptr);
  lph = lptr(lp);
  lptr(lp) = lptr(lph);
  //C
  //C If IO1 is the last neighbor of IO2, make IN1 the
  //C   last neighbor.
  //C
  if (lend(io2) == lph) {
    lend(io2) = lp;
  }
  //C
  //C Insert IN1 as a neighbor of IN2 following IO2.
  //C
  lp = lstptr(lend(in2), io2, list, lptr);
  lpsav = lptr(lp);
  lptr(lp) = lph;
  list(lph) = in1;
  lptr(lph) = lpsav;
  lp21 = lph;
}

bool
swptst(
  common& cmn,
  int const& in1,
  int const& in2,
  int const& io1,
  int const& io2,
  arr_cref<double> x,
  arr_cref<double> y)
{
  bool return_value = fem::bool0;
  x(dimension(star));
  y(dimension(star));
  double dx11 = fem::double0;
  double dx12 = fem::double0;
  double dx22 = fem::double0;
  double dx21 = fem::double0;
  double dy11 = fem::double0;
  double dy12 = fem::double0;
  double dy22 = fem::double0;
  double dy21 = fem::double0;
  double cos1 = fem::double0;
  double cos2 = fem::double0;
  double sin1 = fem::double0;
  double sin2 = fem::double0;
  double sin12 = fem::double0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   09/01/88
  //C
  //C   This function applies the circumcircle test to a quadri-
  //C lateral defined by a pair of adjacent triangles.  The
  //C diagonal arc (shared triangle side) should be swapped for
  //C the other diagonl if and only if the fourth vertex is
  //C strictly interior to the circumcircle of one of the
  //C triangles (the decision is independent of the choice of
  //C triangle).  Equivalently, the diagonal is chosen to maxi-
  //C mize the smallest of the six interior angles over the two
  //C pairs of possible triangles (the decision is for no swap
  //C if the quadrilateral is not strictly convex).
  //C
  //C   When the four vertices are nearly cocircular (the
  //C neutral case), the preferred decision is no swap -- in
  //C order to avoid unnecessary swaps and, more important, to
  //C avoid cycling in Subroutine OPTIM which is called by
  //C DELNOD and EDGE.  Thus, a tolerance SWTOL (stored in
  //C SWPCOM by TRMESH or TRMSHR) is used to define 'nearness'
  //C to the neutral case.
  //C
  //C On input:
  //C
  //C       IN1,IN2,IO1,IO2 = Nodal indexes of the vertices of
  //C                         the quadrilateral.  IO1-IO2 is the
  //C                         triangulation arc (shared triangle
  //C                         side) to be replaced by IN1-IN2 if
  //C                         the decision is to swap.  The
  //C                         triples (IO1,IO2,IN1) and (IO2,
  //C                         IO1,IN2) must define triangles (be
  //C                         in counterclockwise order) on in-
  //C                         put.
  //C
  //C       X,Y = Arrays containing the nodal coordinates.
  //C
  //C Input parameters are not altered by this routine.
  //C
  //C On output:
  //C
  //C       SWPTST = .TRUE. if and only if the arc connecting
  //C                IO1 and IO2 is to be replaced.
  //C
  //C Modules required by SWPTST:  None
  //C
  //C***********************************************************
  //C
  //C Tolerance stored by TRMESH or TRMSHR.
  //C
  //C Local parameters:
  //C
  //C DX11,DY11 = X,Y components of the vector IN1->IO1
  //C DX12,DY12 = X,Y components of the vector IN1->IO2
  //C DX22,DY22 = X,Y components of the vector IN2->IO2
  //C DX21,DY21 = X,Y components of the vector IN2->IO1
  //C SIN1 =      Cross product of the vectors IN1->IO1 and
  //C               IN1->IO2 -- proportional to sin(T1), where
  //C               T1 is the angle at IN1 formed by the vectors
  //C COS1 =      Inner product of the vectors IN1->IO1 and
  //C               IN1->IO2 -- proportional to cos(T1)
  //C SIN2 =      Cross product of the vectors IN2->IO2 and
  //C               IN2->IO1 -- proportional to sin(T2), where
  //C               T2 is the angle at IN2 formed by the vectors
  //C COS2 =      Inner product of the vectors IN2->IO2 and
  //C               IN2->IO1 -- proportional to cos(T2)
  //C SIN12 =     SIN1*COS2 + COS1*SIN2 -- proportional to
  //C               sin(T1+T2)
  //C
  //C Compute the vectors containing the angles T1 and T2.
  //C
  dx11 = x(io1) - x(in1);
  dx12 = x(io2) - x(in1);
  dx22 = x(io2) - x(in2);
  dx21 = x(io1) - x(in2);
  //C
  dy11 = y(io1) - y(in1);
  dy12 = y(io2) - y(in1);
  dy22 = y(io2) - y(in2);
  dy21 = y(io1) - y(in2);
  //C
  //C Compute inner products.
  //C
  cos1 = dx11 * dx12 + dy11 * dy12;
  cos2 = dx22 * dx21 + dy22 * dy21;
  //C
  //C The diagonals should be swapped iff (T1+T2) > 180
  //C   degrees.  The following two tests ensure numerical
  //C   stability:  the decision must be FALSE when both
  //C   angles are close to 0, and TRUE when both angles
  //C   are close to 180 degrees.
  //C
  if (cos1 >= 0.f && cos2 >= 0.f) {
    goto statement_2;
  }
  if (cos1 < 0.f && cos2 < 0.f) {
    goto statement_1;
  }
  //C
  //C Compute vector cross products (Z-components).
  //C
  sin1 = dx11 * dy12 - dx12 * dy11;
  sin2 = dx22 * dy21 - dx21 * dy22;
  sin12 = sin1 * cos2 + cos1 * sin2;
  if (sin12 >=  - cmn.swtol) {
    goto statement_2;
  }
  //C
  //C Swap.
  //C
  statement_1:
  return_value = true;
  return return_value;
  //C
  //C No swap.
  //C
  statement_2:
  return_value = false;
  return return_value;
}

int
jrand(
  int const& n,
  int& ix,
  int& iy,
  int& iz)
{
  int return_value = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                              From STRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   07/28/98
  //C
  //C   This function returns a uniformly distributed pseudo-
  //C random integer in the range 1 to N.
  //C
  //C On input:
  //C
  //C       N = Maximum value to be returned.
  //C
  //C N is not altered by this function.
  //C
  //C       IX,IY,IZ = Integer seeds initialized to values in
  //C                  the range 1 to 30,000 before the first
  //C                  call to JRAND, and not altered between
  //C                  subsequent calls (unless a sequence of
  //C                  random numbers is to be repeated by
  //C                  reinitializing the seeds).
  //C
  //C On output:
  //C
  //C       IX,IY,IZ = Updated integer seeds.
  //C
  //C       JRAND = Random integer in the range 1 to N.
  //C
  //C Reference:  B. A. Wichmann and I. D. Hill, "An Efficient
  //C             and Portable Pseudo-random Number Generator",
  //C             Applied Statistics, Vol. 31, No. 2, 1982,
  //C             pp. 188-190.
  //C
  //C Modules required by JRAND:  None
  //C
  //C Intrinsic functions called by JRAND:  INT, MOD, DBLE
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C U = Pseudo-random number uniformly distributed in the
  //C     interval (0,1).
  //C X = Pseudo-random number in the range 0 to 3 whose frac-
  //C       tional part is U.
  //C
  ix = fem::mod(171 * ix, 30269);
  iy = fem::mod(172 * iy, 30307);
  iz = fem::mod(170 * iz, 30323);
  double x = (fem::dble(ix) / 30269.f) + (fem::dble(iy) / 30307.f) + (
    fem::dble(iz) / 30323.f);
  double u = x - fem::fint(x);
  return_value = fem::dble(n) * u + 1.f;
  return return_value;
}

bool
left(
  double const& x1,
  double const& y1,
  double const& x2,
  double const& y2,
  double const& x0,
  double const& y0)
{
  bool return_value = fem::bool0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   09/01/88
  //C
  //C   This function determines whether node N0 is to the left
  //C or to the right of the line through N1-N2 as viewed by an
  //C observer at N1 facing N2.
  //C
  //C On input:
  //C
  //C       X1,Y1 = Coordinates of N1.
  //C
  //C       X2,Y2 = Coordinates of N2.
  //C
  //C       X0,Y0 = Coordinates of N0.
  //C
  //C Input parameters are not altered by this function.
  //C
  //C On output:
  //C
  //C       LEFT = .TRUE. if and only if (X0,Y0) is on or to the
  //C              left of the directed line N1->N2.
  //C
  //C Modules required by LEFT:  None
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C DX1,DY1 = X,Y components of the vector N1->N2
  //C DX2,DY2 = X,Y components of the vector N1->N0
  //C
  double dx1 = x2 - x1;
  double dy1 = y2 - y1;
  double dx2 = x0 - x1;
  double dy2 = y0 - y1;
  //C
  //C If the sign of the vector cross product of N1->N2 and
  //C   N1->N0 is positive, then sin(A) > 0, where A is the
  //C   angle between the vectors, and thus A is in the range
  //C   (0,180) degrees.
  //C
  return_value = dx1 * dy2 >= dx2 * dy1;
  return return_value;
}

double
store(
  common& cmn,
  double const& x)
{
  double return_value = fem::double0;
  // COMMON stcom
  double& y = cmn.y;
  //
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   03/18/90
  //C
  //C   This function forces its argument X to be stored in a
  //C memory location, thus providing a means of determining
  //C floating point number characteristics (such as the machine
  //C precision) when it is necessary to avoid computation in
  //C high precision registers.
  //C
  //C On input:
  //C
  //C       X = Value to be stored.
  //C
  //C X is not altered by this function.
  //C
  //C On output:
  //C
  //C       STORE = Value of X after it has been stored and
  //C               possibly truncated or rounded to the single
  //C               precision word length.
  //C
  //C Modules required by STORE:  None
  //C
  //C***********************************************************
  //C
  y = x;
  return_value = y;
  return return_value;
}

struct trfind_save
{
  int ix;
  int iy;
  int iz;

  trfind_save() :
    ix(fem::int0),
    iy(fem::int0),
    iz(fem::int0)
  {}
};

void
trfind(
  common& cmn,
  int const& nst,
  double const& px,
  double const& py,
  int const& n,
  arr_cref<double> x,
  arr_cref<double> y,
  arr_cref<int> list,
  arr_cref<int> lptr,
  arr_cref<int> lend,
  int& i1,
  int& i2,
  int& i3)
{
  FEM_CMN_SVE(trfind);
  x(dimension(n));
  y(dimension(n));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  int& ix = sve.ix;
  int& iy = sve.iy;
  int& iz = sve.iz;
  if (is_called_first_time) {
    ix = 1;
    iy = 2;
    iz = 3;
  }
  // double xa = fem::double0;
  // double ya = fem::double0;
  // double xb = fem::double0;
  // double yb = fem::double0;
  // double xc = fem::double0;
  // double yc = fem::double0;
  double xp = fem::double0;
  double yp = fem::double0;
  int n0 = fem::int0;
  int lp = fem::int0;
  int nl = fem::int0;
  int nf = fem::int0;
  int n1 = fem::int0;
  int nb = fem::int0;
  int np = fem::int0;
  int npp = fem::int0;
  int n2 = fem::int0;
  int n3 = fem::int0;
  int n1s = fem::int0;
  int n2s = fem::int0;
  double b1 = fem::double0;
  double b2 = fem::double0;
  int n4 = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   07/28/98
  //C
  //C   This subroutine locates a point P relative to a triangu-
  //C lation created by Subroutine TRMESH or TRMSHR.  If P is
  //C contained in a triangle, the three vertex indexes are
  //C returned.  Otherwise, the indexes of the rightmost and
  //C leftmost visible boundary nodes are returned.
  //C
  //C On input:
  //C
  //C       NST = Index of a node at which TRFIND begins the
  //C             search.  Search time depends on the proximity
  //C             of this node to P.
  //C
  //C       PX,PY = X and y coordinates of the point P to be
  //C               located.
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 3.
  //C
  //C       X,Y = Arrays of length N containing the coordinates
  //C             of the nodes in the triangulation.
  //C
  //C       LIST,LPTR,LEND = Data structure defining the trian-
  //C                        gulation.  Refer to Subroutine
  //C                        TRMESH.
  //C
  //C Input parameters are not altered by this routine.
  //C
  //C On output:
  //C
  //C       I1,I2,I3 = Nodal indexes, in counterclockwise order,
  //C                  of the vertices of a triangle containing
  //C                  P if P is contained in a triangle.  If P
  //C                  is not in the convex hull of the nodes,
  //C                  I1 indexes the rightmost visible boundary
  //C                  node, I2 indexes the leftmost visible
  //C                  boundary node, and I3 = 0.  Rightmost and
  //C                  leftmost are defined from the perspective
  //C                  of P, and a pair of points are visible
  //C                  from each other if and only if the line
  //C                  segment joining them intersects no trian-
  //C                  gulation arc.  If P and all of the nodes
  //C                  lie on a common line, then I1 = I2 = I3 =
  //C                  0 on output.
  //C
  //C Modules required by TRFIND:  JRAND, LEFT, LSTPTR, STORE
  //C
  //C Intrinsic function called by TRFIND:  ABS
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C B1,B2 =    Unnormalized barycentric coordinates of P with
  //C              respect to (N1,N2,N3)
  //C IX,IY,IZ = Integer seeds for JRAND
  //C LP =       LIST pointer
  //C N0,N1,N2 = Nodes in counterclockwise order defining a
  //C              cone (with vertex N0) containing P
  //C N1S,N2S =  Saved values of N1 and N2
  //C N3,N4 =    Nodes opposite N1->N2 and N2->N1, respectively
  //C NB =       Index of a boundary node -- first neighbor of
  //C              NF or last neighbor of NL in the boundary
  //C              traversal loops
  //C NF,NL =    First and last neighbors of N0, or first
  //C              (rightmost) and last (leftmost) nodes
  //C              visible from P when P is exterior to the
  //C              triangulation
  //C NP,NPP =   Indexes of boundary nodes used in the boundary
  //C              traversal loops
  //C XA,XB,XC = Dummy arguments for FRWRD
  //C YA,YB,YC = Dummy arguments for FRWRD
  //C XP,YP =    Local variables containing the components of P
  //C
  //C Statement function:
  //C
  //C FRWRD = TRUE iff C is forward of A->B
  //C              iff <A->B,A->C> .GE. 0.
  //C
  #define frwrd(xa, ya, xb, yb, xc, yc) \
  	((xb - xa) * (xc - xa) + (yb - ya) * (yc - ya) >= 0.f)
  //C
  //C Initialize variables.
  //C
  xp = px;
  yp = py;
  n0 = nst;
  if (n0 < 1 || n0 > n) {
    n0 = jrand(n, ix, iy, iz);
  }
  //C
  //C Set NF and NL to the first and last neighbors of N0, and
  //C   initialize N1 = NF.
  //C
  statement_1:
  lp = lend(n0);
  nl = list(lp);
  lp = lptr(lp);
  nf = list(lp);
  n1 = nf;
  //C
  //C Find a pair of adjacent neighbors N1,N2 of N0 that define
  //C   a wedge containing P:  P LEFT N0->N1 and P RIGHT N0->N2.
  //C
  if (nl > 0) {
    goto statement_2;
  }
  //C
  //C   N0 is a boundary node.  Test for P exterior.
  //C
  nl = -nl;
  if (!left(x(n0), y(n0), x(nf), y(nf), xp, yp)) {
    nl = n0;
    goto statement_9;
  }
  if (!left(x(nl), y(nl), x(n0), y(n0), xp, yp)) {
    nb = nf;
    nf = n0;
    np = nl;
    npp = n0;
    goto statement_11;
  }
  goto statement_3;
  //C
  //C   N0 is an interior node.  Find N1.
  //C
  statement_2:
  if (left(x(n0), y(n0), x(n1), y(n1), xp, yp)) {
    goto statement_3;
  }
  lp = lptr(lp);
  n1 = list(lp);
  if (n1 == nl) {
    goto statement_6;
  }
  goto statement_2;
  //C
  //C   P is to the left of edge N0->N1.  Initialize N2 to the
  //C     next neighbor of N0.
  //C
  statement_3:
  lp = lptr(lp);
  n2 = fem::abs(list(lp));
  if (!left(x(n0), y(n0), x(n2), y(n2), xp, yp)) {
    goto statement_7;
  }
  n1 = n2;
  if (n1 != nl) {
    goto statement_3;
  }
  if (!left(x(n0), y(n0), x(nf), y(nf), xp, yp)) {
    goto statement_6;
  }
  if (xp == x(n0) && yp == y(n0)) {
    goto statement_5;
  }
  //C
  //C   P is left of or on edges N0->NB for all neighbors NB
  //C     of N0.
  //C   All points are collinear iff P is left of NB->N0 for
  //C     all neighbors NB of N0.  Search the neighbors of N0.
  //C     NOTE -- N1 = NL and LP points to NL.
  //C
  statement_4:
  if (!left(x(n1), y(n1), x(n0), y(n0), xp, yp)) {
    goto statement_5;
  }
  lp = lptr(lp);
  n1 = fem::abs(list(lp));
  if (n1 == nl) {
    goto statement_17;
  }
  goto statement_4;
  //C
  //C   P is to the right of N1->N0, or P=N0.  Set N0 to N1 and
  //C     start over.
  //C
  statement_5:
  n0 = n1;
  goto statement_1;
  //C
  //C   P is between edges N0->N1 and N0->NF.
  //C
  statement_6:
  n2 = nf;
  //C
  //C P is contained in the wedge defined by line segments
  //C   N0->N1 and N0->N2, where N1 is adjacent to N2.  Set
  //C   N3 to the node opposite N1->N2, and save N1 and N2 to
  //C   test for cycling.
  //C
  statement_7:
  n3 = n0;
  n1s = n1;
  n2s = n2;
  //C
  //C Top of edge hopping loop.  Test for termination.
  //C
  statement_8:
  if (left(x(n1), y(n1), x(n2), y(n2), xp, yp)) {
    //C
    //C   P LEFT N1->N2 and hence P is in (N1,N2,N3) unless an
    //C     error resulted from floating point inaccuracy and
    //C     collinearity.  Compute the unnormalized barycentric
    //C     coordinates of P with respect to (N1,N2,N3).
    //C
    b1 = (x(n3) - x(n2)) * (yp - y(n2)) - (xp - x(n2)) * (y(n3) - y(n2));
    b2 = (x(n1) - x(n3)) * (yp - y(n3)) - (xp - x(n3)) * (y(n1) - y(n3));
    if (store(cmn, b1 + 1.f) >= 1.f && store(cmn, b2 + 1.f) >= 1.f) {
      goto statement_16;
    }
    //C
    //C   Restart with N0 randomly selected.
    //C
    n0 = jrand(n, ix, iy, iz);
    goto statement_1;
  }
  //C
  //C   Set N4 to the neighbor of N2 which follows N1 (node
  //C     opposite N2->N1) unless N1->N2 is a boundary edge.
  //C
  lp = lstptr(lend(n2), n1, list, lptr);
  if (list(lp) < 0) {
    nf = n2;
    nl = n1;
    goto statement_9;
  }
  lp = lptr(lp);
  n4 = fem::abs(list(lp));
  //C
  //C   Select the new edge N1->N2 which intersects the line
  //C     segment N0-P, and set N3 to the node opposite N1->N2.
  //C
  if (left(x(n0), y(n0), x(n4), y(n4), xp, yp)) {
    n3 = n1;
    n1 = n4;
    n2s = n2;
    if (n1 != n1s && n1 != n0) {
      goto statement_8;
    }
  }
  else {
    n3 = n2;
    n2 = n4;
    n1s = n1;
    if (n2 != n2s && n2 != n0) {
      goto statement_8;
    }
  }
  //C
  //C   The starting node N0 or edge N1-N2 was encountered
  //C     again, implying a cycle (infinite loop).  Restart
  //C     with N0 randomly selected.
  //C
  n0 = jrand(n, ix, iy, iz);
  goto statement_1;
  //C
  //C Boundary traversal loops.  NL->NF is a boundary edge and
  //C   P RIGHT NL->NF.  Save NL and NF.
  //C
  statement_9:
  np = nl;
  npp = nf;
  //C
  //C Find the first (rightmost) visible boundary node NF.  NB
  //C   is set to the first neighbor of NF, and NP is the last
  //C   neighbor.
  //C
  statement_10:
  lp = lend(nf);
  lp = lptr(lp);
  nb = list(lp);
  if (!left(x(nf), y(nf), x(nb), y(nb), xp, yp)) {
    goto statement_12;
  }
  //C
  //C   P LEFT NF->NB and thus NB is not visible unless an error
  //C     resulted from floating point inaccuracy and collinear-
  //C     ity of the 4 points NP, NF, NB, and P.
  //C
  statement_11:
  if (frwrd(x(nf), y(nf), x(np), y(np), xp, yp) || frwrd(x(nf), y(nf),
      x(np), y(np), x(nb), y(nb))) {
    i1 = nf;
    goto statement_13;
  }
  //C
  //C   Bottom of loop.
  //C
  statement_12:
  np = nf;
  nf = nb;
  goto statement_10;
  //C
  //C Find the last (leftmost) visible boundary node NL.  NB
  //C   is set to the last neighbor of NL, and NPP is the first
  //C   neighbor.
  //C
  statement_13:
  lp = lend(nl);
  nb = -list(lp);
  if (!left(x(nb), y(nb), x(nl), y(nl), xp, yp)) {
    goto statement_14;
  }
  //C
  //C   P LEFT NB->NL and thus NB is not visible unless an error
  //C     resulted from floating point inaccuracy and collinear-
  //C     ity of the 4 points P, NB, NL, and NPP.
  //C
  if (frwrd(x(nl), y(nl), x(npp), y(npp), xp, yp) || frwrd(x(nl), y(nl),
      x(npp), y(npp), x(nb), y(nb))) {
    goto statement_15;
  }
  //C
  //C   Bottom of loop.
  //C
  statement_14:
  npp = nl;
  nl = nb;
  goto statement_13;
  //C
  //C NL is the leftmost visible boundary node.
  //C
  statement_15:
  i2 = nl;
  i3 = 0;
  return;
  //C
  //C P is in the triangle (N1,N2,N3).
  //C
  statement_16:
  i1 = n1;
  i2 = n2;
  i3 = n3;
  return;
  //C
  //C All points are collinear.
  //C
  statement_17:
  i1 = 0;
  i2 = 0;
  i3 = 0;
}

void
addnod(
  common& cmn,
  int const& k,
  double const& xk,
  double const& yk,
  int const& ist,
  int const& ncc,
  arr_ref<int> lcc,
  int& n,
  arr_ref<double> x,
  arr_ref<double> y,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& lnew,
  int& ier)
{
  lcc(dimension(star));
  x(dimension(star));
  y(dimension(star));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(star));
  int kk = fem::int0;
  int lccip1 = fem::int0;
  int i = fem::int0;
  int i1 = fem::int0;
  int i2 = fem::int0;
  int i3 = fem::int0;
  int l = fem::int0;
  int nm1 = fem::int0;
  int ibk = fem::int0;
  int lp = fem::int0;
  int lpf = fem::int0;
  int io2 = fem::int0;
  int lpo1 = fem::int0;
  int io1 = fem::int0;
  int in1 = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   06/27/98
  //C
  //C   Given a triangulation of N nodes in the plane created by
  //C Subroutine TRMESH or TRMSHR, this subroutine updates the
  //C data structure with the addition of a new node in position
  //C K.  If node K is inserted into X and Y (K .LE. N) rather
  //C than appended (K = N+1), then a corresponding insertion
  //C must be performed in any additional arrays associated
  //C with the nodes.  For example, an array of data values Z
  //C must be shifted down to open up position K for the new
  //C value:  set Z(I+1) to Z(I) for I = N,N-1,...,K.  For
  //C optimal efficiency, new nodes should be appended whenever
  //C possible.  Insertion is necessary, however, to add a non-
  //C constraint node when constraints are present (refer to
  //C Subroutine ADDCST).
  //C
  //C   Note that a constraint node cannot be added by this
  //C routine.  In order to insert a constraint node, it is
  //C necessary to add the node with no constraints present
  //C (call this routine with NCC = 0), update LCC by increment-
  //C ing the appropriate entries, and then create (or restore)
  //C the constraints by a call to ADDCST.
  //C
  //C   The algorithm consists of the following steps:  node K
  //C is located relative to the triangulation (TRFIND), its
  //C index is added to the data structure (INTADD or BDYADD),
  //C and a sequence of swaps (SWPTST and SWAP) are applied to
  //C the arcs opposite K so that all arcs incident on node K
  //C and opposite node K (excluding constraint arcs) are local-
  //C ly optimal (satisfy the circumcircle test).  Thus, if a
  //C (constrained) Delaunay triangulation is input, a (con-
  //C strained) Delaunay triangulation will result.  All indexes
  //C are incremented as necessary for an insertion.
  //C
  //C On input:
  //C
  //C       K = Nodal index (index for X, Y, and LEND) of the
  //C           new node to be added.  1 .LE. K .LE. LCC(1).
  //C           (K .LE. N+1 if NCC=0).
  //C
  //C       XK,YK = Cartesian coordinates of the new node (to be
  //C               stored in X(K) and Y(K)).  The node must not
  //C               lie in a constraint region.
  //C
  //C       IST = Index of a node at which TRFIND begins the
  //C             search.  Search time depends on the proximity
  //C             of this node to node K.  1 .LE. IST .LE. N.
  //C
  //C       NCC = Number of constraint curves.  NCC .GE. 0.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LCC = List of constraint curve starting indexes (or
  //C             dummy array of length 1 if NCC = 0).  Refer to
  //C             Subroutine ADDCST.
  //C
  //C       N = Number of nodes in the triangulation before K is
  //C           added.  N .GE. 3.  Note that N will be incre-
  //C           mented following the addition of node K.
  //C
  //C       X,Y = Arrays of length at least N+1 containing the
  //C             Cartesian coordinates of the nodes in the
  //C             first N positions with non-constraint nodes
  //C             in the first LCC(1)-1 locations if NCC > 0.
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure associated with
  //C                             the triangulation of nodes 1
  //C                             to N.  The arrays must have
  //C                             sufficient length for N+1
  //C                             nodes.  Refer to TRMESH.
  //C
  //C On output:
  //C
  //C       LCC = List of constraint curve starting indexes in-
  //C             cremented by 1 to reflect the insertion of K
  //C             unless NCC = 0 or (IER .NE. 0 and IER .NE.
  //C             -4).
  //C
  //C       N = Number of nodes in the triangulation including K
  //C           unless IER .NE. 0 and IER .NE. -4.  Note that
  //C           all comments refer to the input value of N.
  //C
  //C       X,Y = Arrays updated with the insertion of XK and YK
  //C             in the K-th positions (node I+1 was node I be-
  //C             fore the insertion for I = K to N if K .LE. N)
  //C             unless IER .NE. 0 and IER .NE. -4.
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure updated with
  //C                             the addition of node K unless
  //C                             IER .NE. 0 and IER .NE. -4.
  //C
  //C       IER = Error indicator:
  //C             IER =  0 if no errors were encountered.
  //C             IER = -1 if K, IST, NCC, N, or an LCC entry is
  //C                      outside its valid range on input.
  //C             IER = -2 if all nodes (including K) are col-
  //C                      linear.
  //C             IER =  L if nodes L and K coincide for some L.
  //C             IER = -3 if K lies in a constraint region.
  //C             IER = -4 if an error flag is returned by SWAP
  //C                      implying that the triangulation
  //C                      (geometry) was bad on input.
  //C
  //C             The errors conditions are tested in the order
  //C             specified.
  //C
  //C Modules required by ADDNOD:  BDYADD, CRTRI, INDXCC,
  //C                                INSERT, INTADD, JRAND,
  //C                                LEFT, LSTPTR, SWAP,
  //C                                SWPTST, TRFIND
  //C
  //C Intrinsic function called by ADDNOD:  ABS
  //C
  //C***********************************************************
  //C
  kk = k;
  //C
  //C Test for an invalid input parameter.
  //C
  if (kk < 1 || ist < 1 || ist > n || ncc < 0 || n < 3) {
    goto statement_7;
  }
  lccip1 = n + 1;
  FEM_DOSTEP(i, ncc, 1, -1) {
    if (lccip1 - lcc(i) < 3) {
      goto statement_7;
    }
    lccip1 = lcc(i);
  }
  if (kk > lccip1) {
    goto statement_7;
  }
  //C
  //C Find a triangle (I1,I2,I3) containing K or the rightmost
  //C   (I1) and leftmost (I2) visible boundary nodes as viewed
  //C   from node K.
  //C
  trfind(cmn, ist, xk, yk, n, x, y, list, lptr, lend, i1, i2, i3);
  //C
  //C Test for collinear nodes, duplicate nodes, and K lying in
  //C   a constraint region.
  //C
  if (i1 == 0) {
    goto statement_8;
  }
  if (i3 != 0) {
    l = i1;
    if (xk == x(l) && yk == y(l)) {
      goto statement_9;
    }
    l = i2;
    if (xk == x(l) && yk == y(l)) {
      goto statement_9;
    }
    l = i3;
    if (xk == x(l) && yk == y(l)) {
      goto statement_9;
    }
    if (ncc > 0 && crtri(ncc, lcc, i1, i2, i3)) {
      goto statement_10;
    }
  }
  else {
    //C
    //C   K is outside the convex hull of the nodes and lies in a
    //C     constraint region iff an exterior constraint curve is
    //C     present.
    //C
    if (ncc > 0 && indxcc(ncc, lcc, n, list, lend) != 0) {
      goto statement_10;
    }
  }
  //C
  //C No errors encountered.
  //C
  ier = 0;
  nm1 = n;
  n++;
  if (kk < n) {
    //C
    //C Open a slot for K in X, Y, and LEND, and increment all
    //C   nodal indexes which are greater than or equal to K.
    //C   Note that LIST, LPTR, and LNEW are not yet updated with
    //C   either the neighbors of K or the edges terminating on K.
    //C
    FEM_DOSTEP(ibk, nm1, kk, -1) {
      x(ibk + 1) = x(ibk);
      y(ibk + 1) = y(ibk);
      lend(ibk + 1) = lend(ibk);
    }
    FEM_DO_SAFE(i, 1, ncc) {
      lcc(i)++;
    }
    l = lnew - 1;
    FEM_DO_SAFE(i, 1, l) {
      if (list(i) >= kk) {
        list(i)++;
      }
      if (list(i) <=  - kk) {
        list(i) = list(i) - 1;
      }
    }
    if (i1 >= kk) {
      i1++;
    }
    if (i2 >= kk) {
      i2++;
    }
    if (i3 >= kk) {
      i3++;
    }
  }
  //C
  //C Insert K into X and Y, and update LIST, LPTR, LEND, and
  //C   LNEW with the arcs containing node K.
  //C
  x(kk) = xk;
  y(kk) = yk;
  if (i3 == 0) {
    bdyadd(kk, i1, i2, list, lptr, lend, lnew);
  }
  else {
    intadd(kk, i1, i2, i3, list, lptr, lend, lnew);
  }
  //C
  //C Initialize variables for optimization of the triangula-
  //C   tion.
  //C
  lp = lend(kk);
  lpf = lptr(lp);
  io2 = list(lpf);
  lpo1 = lptr(lpf);
  io1 = fem::abs(list(lpo1));
  //C
  //C Begin loop:  find the node opposite K.
  //C
  statement_5:
  lp = lstptr(lend(io1), io2, list, lptr);
  if (list(lp) < 0) {
    goto statement_6;
  }
  lp = lptr(lp);
  in1 = fem::abs(list(lp));
  if (crtri(ncc, lcc, io1, io2, in1)) {
    goto statement_6;
  }
  //C
  //C Swap test:  if a swap occurs, two new arcs are
  //C             opposite K and must be tested.
  //C
  if (!swptst(cmn, in1, kk, io1, io2, x, y)) {
    goto statement_6;
  }
  swap(in1, kk, io1, io2, list, lptr, lend, lpo1);
  if (lpo1 == 0) {
    goto statement_11;
  }
  io1 = in1;
  goto statement_5;
  //C
  //C No swap occurred.  Test for termination and reset
  //C   IO2 and IO1.
  //C
  statement_6:
  if (lpo1 == lpf || list(lpo1) < 0) {
    return;
  }
  io2 = io1;
  lpo1 = lptr(lpo1);
  io1 = fem::abs(list(lpo1));
  goto statement_5;
  //C
  //C A parameter is outside its valid range on input.
  //C
  statement_7:
  ier = -1;
  return;
  //C
  //C All nodes are collinear.
  //C
  statement_8:
  ier = -2;
  return;
  //C
  //C Nodes L and K coincide.
  //C
  statement_9:
  ier = l;
  return;
  //C
  //C Node K lies in a constraint region.
  //C
  statement_10:
  ier = -3;
  return;
  //C
  //C Zero pointer returned by SWAP.
  //C
  statement_11:
  ier = -4;
}

void
trmesh(
  common& cmn,
  int const& n,
  arr_ref<double> x,
  arr_ref<double> y,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& lnew,
  arr_ref<int> near,
  arr_ref<int> next,
  arr_ref<double> dist,
  int& ier)
{
  x(dimension(n));
  y(dimension(n));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  near(dimension(n));
  next(dimension(n));
  dist(dimension(n));
  double& swtol = cmn.swtol;
  //
  int nn = fem::int0;
  double eps = fem::double0;
  int k = fem::int0;
  double d1 = fem::double0;
  double d2 = fem::double0;
  double d3 = fem::double0;
  int ncc = fem::int0;
  int km1 = fem::int0;
  arr_1d<1, int> lcc(fem::fill0);
  int i = fem::int0;
  int i0 = fem::int0;
  int lpl = fem::int0;
  int lp = fem::int0;
  int j = fem::int0;
  int nexti = fem::int0;
  double d = fem::double0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   06/28/98
  //C
  //C   This subroutine creates a Delaunay triangulation of a
  //C set of N arbitrarily distributed points in the plane re-
  //C ferred to as nodes.  The Delaunay triangulation is defined
  //C as a set of triangles with the following five properties:
  //C
  //C  1)  The triangle vertices are nodes.
  //C  2)  No triangle contains a node other than its vertices.
  //C  3)  The interiors of the triangles are pairwise disjoint.
  //C  4)  The union of triangles is the convex hull of the set
  //C        of nodes (the smallest convex set which contains
  //C        the nodes).
  //C  5)  The interior of the circumcircle of each triangle
  //C        contains no node.
  //C
  //C The first four properties define a triangulation, and the
  //C last property results in a triangulation which is as close
  //C as possible to equiangular in a certain sense and which is
  //C uniquely defined unless four or more nodes lie on a common
  //C circle.  This property makes the triangulation well-suited
  //C for solving closest point problems and for triangle-based
  //C interpolation.
  //C
  //C   The triangulation can be generalized to a constrained
  //C Delaunay triangulation by a call to Subroutine ADDCST.
  //C This allows for user-specified boundaries defining a non-
  //C convex and/or multiply connected region.
  //C
  //C   The algorithm for constructing the triangulation has
  //C expected time complexity O(N*log(N)) for most nodal dis-
  //C tributions.  Also, since the algorithm proceeds by adding
  //C nodes incrementally, the triangulation may be updated with
  //C the addition (or deletion) of a node very efficiently.
  //C The adjacency information representing the triangulation
  //C is stored as a linked list requiring approximately 13N
  //C storage locations.
  //C
  //C   The following is a list of the software package modules
  //C which a user may wish to call directly:
  //C
  //C  ADDCST - Generalizes the Delaunay triangulation to allow
  //C             for user-specified constraints.
  //C
  //C  ADDNOD - Updates the triangulation by appending or
  //C             inserting a new node.
  //C
  //C  AREAP  - Computes the area bounded by a closed polygonal
  //C             curve such as the boundary of the triangula-
  //C             tion or of a constraint region.
  //C
  //C  BNODES - Returns an array containing the indexes of the
  //C             boundary nodes in counterclockwise order.
  //C             Counts of boundary nodes, triangles, and arcs
  //C             are also returned.
  //C
  //C  CIRCUM - Computes the area, circumcenter, circumradius,
  //C             and, optionally, the aspect ratio of a trian-
  //C             gle defined by user-specified vertices.
  //C
  //C  DELARC - Deletes a boundary arc from the triangulation.
  //C
  //C  DELNOD - Updates the triangulation with the deletion of a
  //C             node.
  //C
  //C  EDGE   - Forces a pair of nodes to be connected by an arc
  //C             in the triangulation.
  //C
  //C  GETNP  - Determines the ordered sequence of L closest
  //C             nodes to a given node, along with the associ-
  //C             ated distances.  The distance between nodes is
  //C             taken to be the length of the shortest connec-
  //C             ting path which intersects no constraint
  //C             region.
  //C
  //C  INTSEC - Determines whether or not an arbitrary pair of
  //C             line segments share a common point.
  //C
  //C  JRAND  - Generates a uniformly distributed pseudo-random
  //C             integer.
  //C
  //C  LEFT   - Locates a point relative to a line.
  //C
  //C  NEARND - Returns the index of the nearest node to an
  //C             arbitrary point, along with its squared
  //C             distance.
  //C
  //C  STORE  - Forces a value to be stored in main memory so
  //C             that the precision of floating point numbers
  //C             in memory locations rather than registers is
  //C             computed.
  //C
  //C  TRLIST - Converts the triangulation data structure to a
  //C             triangle list more suitable for use in a fin-
  //C             ite element code.
  //C
  //C  TRLPRT - Prints the triangle list created by Subroutine
  //C             TRLIST.
  //C
  //C  TRMESH - Creates a Delaunay triangulation of a set of
  //C             nodes.
  //C
  //C  TRMSHR - Creates a Delaunay triangulation (more effici-
  //C             ently than TRMESH) of a set of nodes lying at
  //C             the vertices of a (possibly skewed) rectangu-
  //C             lar grid.
  //C
  //C  TRPLOT - Creates a level-2 Encapsulated Postscript (EPS)
  //C             file containing a triangulation plot.
  //C
  //C  TRPRNT - Prints the triangulation data structure and,
  //C             optionally, the nodal coordinates.
  //C
  //C On input:
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 3.
  //C
  //C       X,Y = Arrays of length N containing the Cartesian
  //C             coordinates of the nodes.  (X(K),Y(K)) is re-
  //C             ferred to as node K, and K is referred to as
  //C             a nodal index.  The first three nodes must not
  //C             be collinear.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LIST,LPTR = Arrays of length at least 6N-12.
  //C
  //C       LEND = Array of length at least N.
  //C
  //C       NEAR,NEXT,DIST = Work space arrays of length at
  //C                        least N.  The space is used to
  //C                        efficiently determine the nearest
  //C                        triangulation node to each un-
  //C                        processed node for use by ADDNOD.
  //C
  //C On output:
  //C
  //C       LIST = Set of nodal indexes which, along with LPTR,
  //C              LEND, and LNEW, define the triangulation as a
  //C              set of N adjacency lists -- counterclockwise-
  //C              ordered sequences of neighboring nodes such
  //C              that the first and last neighbors of a bound-
  //C              ary node are boundary nodes (the first neigh-
  //C              bor of an interior node is arbitrary).  In
  //C              order to distinguish between interior and
  //C              boundary nodes, the last neighbor of each
  //C              boundary node is represented by the negative
  //C              of its index.
  //C
  //C       LPTR = Set of pointers (LIST indexes) in one-to-one
  //C              correspondence with the elements of LIST.
  //C              LIST(LPTR(I)) indexes the node which follows
  //C              LIST(I) in cyclical counterclockwise order
  //C              (the first neighbor follows the last neigh-
  //C              bor).
  //C
  //C       LEND = Set of pointers to adjacency lists.  LEND(K)
  //C              points to the last neighbor of node K for
  //C              K = 1,...,N.  Thus, LIST(LEND(K)) < 0 if and
  //C              only if K is a boundary node.
  //C
  //C       LNEW = Pointer to the first empty location in LIST
  //C              and LPTR (list length plus one).  LIST, LPTR,
  //C              LEND, and LNEW are not altered if IER < 0,
  //C              and are incomplete if IER > 0.
  //C
  //C       NEAR,NEXT,DIST = Garbage.
  //C
  //C       IER = Error indicator:
  //C             IER =  0 if no errors were encountered.
  //C             IER = -1 if N < 3 on input.
  //C             IER = -2 if the first three nodes are
  //C                      collinear.
  //C             IER = -4 if an error flag was returned by a
  //C                      call to SWAP in ADDNOD.  This is an
  //C                      internal error and should be reported
  //C                      to the programmer.
  //C             IER =  L if nodes L and M coincide for some
  //C                      M > L.  The linked list represents
  //C                      a triangulation of nodes 1 to M-1
  //C                      in this case.
  //C
  //C Modules required by TRMESH:  ADDNOD, BDYADD, INSERT,
  //C                                INTADD, JRAND, LEFT,
  //C                                LSTPTR, STORE, SWAP,
  //C                                SWPTST, TRFIND
  //C
  //C Intrinsic function called by TRMESH:  ABS
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C D =        Squared distance from node K to node I
  //C D1,D2,D3 = Squared distances from node K to nodes 1, 2,
  //C              and 3, respectively
  //C EPS =      Half the machine precision
  //C I,J =      Nodal indexes
  //C I0 =       Index of the node preceding I in a sequence of
  //C              unprocessed nodes:  I = NEXT(I0)
  //C K =        Index of node to be added and DO-loop index:
  //C              K > 3
  //C KM1 =      K-1
  //C LCC(1) =   Dummy array
  //C LP =       LIST index (pointer) of a neighbor of K
  //C LPL =      Pointer to the last neighbor of K
  //C NCC =      Number of constraint curves
  //C NEXTI =    NEXT(I)
  //C NN =       Local copy of N
  //C SWTOL =    Tolerance for function SWPTST
  //C
  nn = n;
  if (nn < 3) {
    ier = -1;
    return;
  }
  //C
  //C Compute a tolerance for function SWPTST:  SWTOL = 10*
  //C   (machine precision)
  //C
  eps = 1.f;
  statement_1:
  eps = eps / 2.f;
  swtol = store(cmn, eps + 1.f);
  if (swtol > 1.f) {
    goto statement_1;
  }
  swtol = eps * 20.f;
  //C
  //C Store the first triangle in the linked list.
  //C
  if (!left(x(1), y(1), x(2), y(2), x(3), y(3))) {
    //C
    //C   The initial triangle is (3,2,1) = (2,1,3) = (1,3,2).
    //C
    list(1) = 3;
    lptr(1) = 2;
    list(2) = -2;
    lptr(2) = 1;
    lend(1) = 2;
    //C
    list(3) = 1;
    lptr(3) = 4;
    list(4) = -3;
    lptr(4) = 3;
    lend(2) = 4;
    //C
    list(5) = 2;
    lptr(5) = 6;
    list(6) = -1;
    lptr(6) = 5;
    lend(3) = 6;
    //C
  }
  else if (!left(x(2), y(2), x(1), y(1), x(3), y(3))) {
    //C
    //C   The initial triangle is (1,2,3).
    //C
    list(1) = 2;
    lptr(1) = 2;
    list(2) = -3;
    lptr(2) = 1;
    lend(1) = 2;
    //C
    list(3) = 3;
    lptr(3) = 4;
    list(4) = -1;
    lptr(4) = 3;
    lend(2) = 4;
    //C
    list(5) = 1;
    lptr(5) = 6;
    list(6) = -2;
    lptr(6) = 5;
    lend(3) = 6;
    //C
  }
  else {
    //C
    //C   The first three nodes are collinear.
    //C
    ier = -2;
    return;
  }
  //C
  //C Initialize LNEW and test for N = 3.
  //C
  lnew = 7;
  if (nn == 3) {
    ier = 0;
    return;
  }
  //C
  //C A nearest-node data structure (NEAR, NEXT, and DIST) is
  //C   used to obtain an expected-time (N*log(N)) incremental
  //C   algorithm by enabling constant search time for locating
  //C   each new node in the triangulation.
  //C
  //C For each unprocessed node K, NEAR(K) is the index of the
  //C   triangulation node closest to K (used as the starting
  //C   point for the search in Subroutine TRFIND) and DIST(K)
  //C   is an increasing function of the distance between nodes
  //C   K and NEAR(K).
  //C
  //C Since it is necessary to efficiently find the subset of
  //C   unprocessed nodes associated with each triangulation
  //C   node J (those that have J as their NEAR entries), the
  //C   subsets are stored in NEAR and NEXT as follows:  for
  //C   each node J in the triangulation, I = NEAR(J) is the
  //C   first unprocessed node in J's set (with I = 0 if the
  //C   set is empty), L = NEXT(I) (if I > 0) is the second,
  //C   NEXT(L) (if L > 0) is the third, etc.  The nodes in each
  //C   set are initially ordered by increasing indexes (which
  //C   maximizes efficiency) but that ordering is not main-
  //C   tained as the data structure is updated.
  //C
  //C Initialize the data structure for the single triangle.
  //C
  near(1) = 0;
  near(2) = 0;
  near(3) = 0;
  FEM_DOSTEP(k, nn, 4, -1) {
    d1 = fem::pow2((x(k) - x(1))) + fem::pow2((y(k) - y(1)));
    d2 = fem::pow2((x(k) - x(2))) + fem::pow2((y(k) - y(2)));
    d3 = fem::pow2((x(k) - x(3))) + fem::pow2((y(k) - y(3)));
    if (d1 <= d2 && d1 <= d3) {
      near(k) = 1;
      dist(k) = d1;
      next(k) = near(1);
      near(1) = k;
    }
    else if (d2 <= d1 && d2 <= d3) {
      near(k) = 2;
      dist(k) = d2;
      next(k) = near(2);
      near(2) = k;
    }
    else {
      near(k) = 3;
      dist(k) = d3;
      next(k) = near(3);
      near(3) = k;
    }
  }
  //C
  //C Add the remaining nodes.  Parameters for ADDNOD are as
  //C   follows:
  //C
  //C   K = Index of the node to be added.
  //C   NEAR(K) = Index of the starting node for the search in
  //C             TRFIND.
  //C   NCC = Number of constraint curves.
  //C   LCC = Dummy array (since NCC = 0).
  //C   KM1 = Number of nodes in the triangulation.
  //C
  ncc = 0;
  FEM_DO_SAFE(k, 4, nn) {
    km1 = k - 1;
    addnod(cmn, k, x(k), y(k), near(k), ncc, lcc, km1, x, y, list,
      lptr, lend, lnew, ier);
    if (ier != 0) {
      return;
    }
    //C
    //C Remove K from the set of unprocessed nodes associated
    //C   with NEAR(K).
    //C
    i = near(k);
    if (near(i) == k) {
      near(i) = next(k);
    }
    else {
      i = near(i);
      statement_3:
      i0 = i;
      i = next(i0);
      if (i != k) {
        goto statement_3;
      }
      next(i0) = next(k);
    }
    near(k) = 0;
    //C
    //C Loop on neighbors J of node K.
    //C
    lpl = lend(k);
    lp = lpl;
    statement_4:
    lp = lptr(lp);
    j = fem::abs(list(lp));
    //C
    //C Loop on elements I in the sequence of unprocessed nodes
    //C   associated with J:  K is a candidate for replacing J
    //C   as the nearest triangulation node to I.  The next value
    //C   of I in the sequence, NEXT(I), must be saved before I
    //C   is moved because it is altered by adding I to K's set.
    //C
    i = near(j);
    statement_5:
    if (i == 0) {
      goto statement_6;
    }
    nexti = next(i);
    //C
    //C Test for the distance from I to K less than the distance
    //C   from I to J.
    //C
    d = fem::pow2((x(k) - x(i))) + fem::pow2((y(k) - y(i)));
    if (d < dist(i)) {
      //C
      //C Replace J by K as the nearest triangulation node to I:
      //C   update NEAR(I) and DIST(I), and remove I from J's set
      //C   of unprocessed nodes and add it to K's set.
      //C
      near(i) = k;
      dist(i) = d;
      if (i == near(j)) {
        near(j) = nexti;
      }
      else {
        next(i0) = nexti;
      }
      next(i) = near(k);
      near(k) = i;
    }
    else {
      i0 = i;
    }
    //C
    //C Bottom of loop on I.
    //C
    i = nexti;
    goto statement_5;
    //C
    //C Bottom of loop on neighbors J.
    //C
    statement_6:
    if (lp != lpl) {
      goto statement_4;
    }
  }
}

void
sdtrch(
  common& cmn,
  int const& ndp,
  arr_ref<double> xd,
  arr_ref<double> yd,
  int& nt,
  arr_ref<int, 2> ipt,
  int& nl,
  arr_ref<int, 2> ipl,
  int& iertm,
  int& iertl,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  arr_ref<int, 2> ltri,
  arr_ref<int> near,
  arr_ref<int> next,
  arr_ref<double> dist)
{
  const int nrow = 6;
  xd(dimension(ndp));
  yd(dimension(ndp));
  ipt(dimension(3, star));
  ipl(dimension(2, star));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(ndp));
  ltri(dimension(nrow, star));
  near(dimension(ndp));
  next(dimension(ndp));
  dist(dimension(ndp));
  int lnew = fem::int0;
  const int ncc = 0;
  arr_1d<1, int> lcc(fem::fill0);
  arr_1d<1, int> lct(fem::fill0);
  int j = fem::int0;
  int i = fem::int0;
  int il = fem::int0;
  int i1 = fem::int0;
  int i2 = fem::int0;
  int il1 = fem::int0;
  int il2 = fem::int0;
  int ipl11 = fem::int0;
  int ipl21 = fem::int0;
  //C
  //C Basic triangulation in the convex hull of a scattered data point
  //C set in a plane
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine triangulates the data area that is a convex hull
  //C of the scattered data points in the x-y plane.  It divides the
  //C data area into a number of triangles and determines line segments
  //C that form the border of the data area.
  //C
  //C This subroutine depends on the TRIPACK package of ACM Algorithm
  //C 751 by R. J. Renka.  It calls the TRMESH and TRLIST subroutines
  //C included in the package.  The TRMESH subroutine in turn calls
  //C either directly or indirectly 12 other subprograms included in
  //C the package.
  //C
  //C The input arguments are
  //C   NDP   = number of data points (must be greater than 3),
  //C   XD    = array of dimension NDP containing the x
  //C           coordinates of the data points,
  //C   YD    = array of dimension NDP containing the y
  //C           coordinates of the data points.
  //C
  //C The output arguments are
  //C   NT    = number of triangles (its maximum is 2*NDP-5),
  //C   IPT   = two-dimensional integer array of dimension
  //C           (3,NT), where the point numbers of the vertexes
  //C           of the ITth triangle are to be stored counter-
  //C           clockwise in the ITth column, where IT = 1, 2,
  //C           ..., NT,
  //C   NL    = number of border line segments (its maximum is
  //C           NDP),
  //C   IPL   = two-dimensional integer array of dimension
  //C           (2,NL), where the point numbers of the end
  //C           points of the (IL)th border line segment are to
  //C           be stored counterclockwise in the ILth column,
  //C           where IL = 1, 2, ..., NL, with the line segments
  //C           stored counterclockwise,
  //C   IERTM = error flag from the TRMESH subroutine,
  //C         =  0 for no errors
  //C         = -1 for NDP = 3 or less
  //C         = -2 for the first three collinear data points,
  //C         =  L for the Lth data point identical to some
  //C            Mth data point, M > L.
  //C   IERTL = error flag from the TRLIST subroutine,
  //C         = 0 for no errors
  //C         = 1 for invalid NCC, NDP, or NROW value.
  //C         = 2 for invalid data structure (LIST,LPTR,LEND).
  //C
  //C The other arguments are
  //C   LIST  = integer array of dimension 6*NDP USED internally
  //C           as a work area,
  //C   LPTR  = integer array of dimension 6*NDP USED internally
  //C           as a work area,
  //C   LEND  = integer array of dimension NDP USED internally as
  //C           a work area,
  //C   LTRI  = two-dimensional integer array of dimension 12*NDP
  //C           used internally as a work area.
  //C
  //C agebhard@uni-klu.ac.at: added from new TRIPACK:
  //C   NEAR, NEXT, DIST work arrays from TRMESH, size NDP
  //C
  //C Specification statements
  //C     .. Parameters ..
  //C     ..
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Local Arrays ..
  //C     ..
  //C     .. External Subroutines ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C Performs basic triangulation.
  trmesh(cmn, ndp, xd, yd, list, lptr, lend, lnew, near, next, dist, iertm);
  if (iertm != 0) {
    return;
  }
  trlist(ncc, lcc, ndp, list, lptr, lend, nrow, nt, ltri, lct, iertl);
  if (iertl != 0) {
    return;
  }
  //C Extracts the triangle data from the LTRI array and set the IPT
  //C array.
  FEM_DO_SAFE(j, 1, nt) {
    FEM_DO_SAFE(i, 1, 3) {
      ipt(i, j) = ltri(i, j);
    }
  }
  //C Extracts the border-line-segment data from the LTRI array and
  //C set the IPL array.
  il = 0;
  FEM_DO_SAFE(j, 1, nt) {
    FEM_DO_SAFE(i, 1, 3) {
      if (ltri(i + 3, j) <= 0) {
        goto statement_40;
      }
    }
    goto statement_50;
    statement_40:
    il++;
    i1 = fem::mod(i, 3) + 1;
    i2 = fem::mod(i + 1, 3) + 1;
    ipl(1, il) = ltri(i1, j);
    ipl(2, il) = ltri(i2, j);
    statement_50:;
  }
  nl = il;
  //C Sorts the IPL array.
  FEM_DO_SAFE(il1, 1, nl - 1) {
    FEM_DO_SAFE(il2, il1 + 1, nl) {
      if (ipl(1, il2) == ipl(2, il1)) {
        goto statement_70;
      }
    }
    statement_70:
    ipl11 = ipl(1, il1 + 1);
    ipl21 = ipl(2, il1 + 1);
    ipl(1, il1 + 1) = ipl(1, il2);
    ipl(2, il1 + 1) = ipl(2, il2);
    ipl(1, il2) = ipl11;
    ipl(2, il2) = ipl21;
  }
}

void
sdtrtt(
  int const& ndp,
  arr_cref<double> xd,
  arr_cref<double> yd,
  int& nt,
  arr_ref<int, 2> ipt,
  int& nl,
  arr_ref<int, 2> ipl,
  arr_ref<int> itl)
{
  xd(dimension(ndp));
  yd(dimension(ndp));
  ipt(dimension(3, star));
  ipl(dimension(2, star));
  itl(dimension(ndp));
  // double u1 = fem::double0;
  // double v1 = fem::double0;
  // double u2 = fem::double0;
  // double v2 = fem::double0;
  // double u3 = fem::double0;
  // double v3 = fem::double0;
  int il = fem::int0;
  int ipl1 = fem::int0;
  int ipl2 = fem::int0;
  int it = fem::int0;
  int irep = fem::int0;
  const int nrrtt = 5;
  int modif = fem::int0;
  int nl0 = fem::int0;
  int il0 = fem::int0;
  int ip1 = fem::int0;
  int ip2 = fem::int0;
  int ip3 = fem::int0;
  double hbr = fem::double0;
  const double hbrmn = 0.10f;
  int itp1 = fem::int0;
  int it0 = fem::int0;
  int il00 = fem::int0;
  int ilp1 = fem::int0;
  int ilr1 = fem::int0;
  int il1 = fem::int0;
  int iv = fem::int0;
  int ivp1 = fem::int0;
  //C
  //C Removal of thin triangles along the border line of triangulation
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine removes thin triangles along the border line of
  //C triangulation.
  //C
  //C The input arguments are
  //C   NDP = number of data points (must be greater than 3),
  //C   XD  = array of dimension NDP containing the x
  //C         coordinates of the data points,
  //C   YD  = array of dimension NDP containing the y
  //C         coordinates of the data points.
  //C
  //C The input and output arguments are
  //C   NT  = number of triangles (its maximum is 2*NDP-5),
  //C   IPT = two-dimensional integer array of dimension
  //C         (3,NT), where the point numbers of the vertexes
  //C         of the ITth triangle are to be stored counter-
  //C         clockwise in the ITth column, where IT = 1, 2,
  //C         ..., NT,
  //C   NL  = number of border line segments (its maximum is
  //C         NDP),
  //C   IPL = two-dimensional integer array of dimension
  //C         (2,NL), where the point numbers of the end
  //C         points of the (IL)th border line segment are to
  //C         be stored counterclockwise in the ILth column,
  //C         where IL = 1, 2, ..., NL, with the line segments
  //C         stored counterclockwise.
  //C
  //C The other argument is
  //C   ITL = integer array of dimension NDP used internally as
  //C         a work area.
  //C
  //C The constants in the PARAMETER statement below are
  //C   HBRMN = minimum value of the height-to-bottom ratio of a
  //C           triangle along the border line of the data area,
  //C   NRRTT = number of repetitions in thin triangle removal.
  //C The constant values have been selected empirically.
  //C
  //C Specification statements
  //C     .. Parameters ..
  //C     ..
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C     .. Statement Functions ..
  //C     ..
  //C Statement Function definitions
  #define dsqf(u1, v1, u2, v2) fem::pow2((u2 - u1)) + fem::pow2((v2 - v1))
  #define vpdt(u1, v1, u2, v2, u3, v3) (v3 - v1) * (u2 - u1) - (u3 - u1) * (v2 - v1)
  //C     ..
  //C Triangle numbers of triangles that share line segments with the
  //C border line.
  FEM_DO_SAFE(il, 1, nl) {
    ipl1 = ipl(1, il);
    ipl2 = ipl(2, il);
    FEM_DO_SAFE(it, 1, nt) {
      if (ipl1 == ipt(1, it) || ipl1 == ipt(2, it) || ipl1 == ipt(3, it)) {
        if (ipl2 == ipt(1, it) || ipl2 == ipt(2, it) || ipl2 == ipt(3, it)) {
          itl(il) = it;
          goto statement_20;
        }
      }
    }
    statement_20:;
  }
  //C Removes thin triangles that share line segments with the border
  //C line.
  FEM_DO_SAFE(irep, 1, nrrtt) {
    modif = 0;
    nl0 = nl;
    il = 0;
    FEM_DO_SAFE(il0, 1, nl0) {
      il++;
      ip1 = ipl(1, il);
      ip2 = ipl(2, il);
      it = itl(il);
      //C Calculates the height-to-bottom ratio of the triangle.
      if (ipt(1, it) != ip1 && ipt(1, it) != ip2) {
        ip3 = ipt(1, it);
      }
      else if (ipt(2, it) != ip1 && ipt(2, it) != ip2) {
        ip3 = ipt(2, it);
      }
      else {
        ip3 = ipt(3, it);
      }
      hbr = vpdt(xd(ip1), yd(ip1), xd(ip2), yd(ip2), xd(ip3), yd(
        ip3)) / dsqf(xd(ip1), yd(ip1), xd(ip2), yd(ip2));
      if (hbr < hbrmn) {
        modif = 1;
        //C Removes this triangle when applicable.
        itp1 = it + 1;
        FEM_DO_SAFE(it0, itp1, nt) {
          ipt(1, it0 - 1) = ipt(1, it0);
          ipt(2, it0 - 1) = ipt(2, it0);
          ipt(3, it0 - 1) = ipt(3, it0);
        }
        nt = nt - 1;
        FEM_DO_SAFE(il00, 1, nl) {
          if (itl(il00) > it) {
            itl(il00) = itl(il00) - 1;
          }
        }
        //C Replaces the border line segment with two new line segments.
        if (il < nl) {
          ilp1 = il + 1;
          FEM_DO_SAFE(ilr1, ilp1, nl) {
            il1 = nl + ilp1 - ilr1;
            ipl(1, il1 + 1) = ipl(1, il1);
            ipl(2, il1 + 1) = ipl(2, il1);
            itl(il1 + 1) = itl(il1);
          }
        }
        //C - Adds the first new line segment.
        ipl(1, il) = ip1;
        ipl(2, il) = ip3;
        FEM_DO_SAFE(it0, 1, nt) {
          FEM_DO_SAFE(iv, 1, 3) {
            if (ipt(iv, it0) == ip1 || ipt(iv, it0) == ip3) {
              ivp1 = fem::mod(iv, 3) + 1;
              if (ipt(ivp1, it0) == ip1 || ipt(ivp1, it0) == ip3) {
                goto statement_80;
              }
            }
          }
        }
        statement_80:
        itl(il) = it0;
        //C - Adds the second new line segment.
        il++;
        ipl(1, il) = ip3;
        ipl(2, il) = ip2;
        FEM_DO_SAFE(it0, 1, nt) {
          FEM_DO_SAFE(iv, 1, 3) {
            if (ipt(iv, it0) == ip3 || ipt(iv, it0) == ip2) {
              ivp1 = fem::mod(iv, 3) + 1;
              if (ipt(ivp1, it0) == ip3 || ipt(ivp1, it0) == ip2) {
                goto statement_110;
              }
            }
          }
        }
        statement_110:
        itl(il) = it0;
        nl++;
      }
    }
    if (modif == 0) {
      return;
    }
  }
  #undef dsqf
  #undef vpdt
}

void
sdtran(
  common& cmn,
  int const& ndp,
  arr_ref<double> xd,
  arr_ref<double> yd,
  int& nt,
  arr_ref<int, 2> ipt,
  int& nl,
  arr_ref<int, 2> ipl,
  int& iert,
  arr_ref<int, 2> list,
  arr_ref<int, 2> lptr,
  arr_ref<int> lend,
  arr_ref<int, 2> ltri,
  arr_ref<int> itl,
  arr_ref<int> near,
  arr_ref<int> next,
  arr_ref<double> dist)
{
  xd(dimension(ndp));
  yd(dimension(ndp));
  ipt(dimension(3, star));
  ipl(dimension(2, star));
  list(dimension(6, ndp));
  lptr(dimension(6, ndp));
  lend(dimension(ndp));
  ltri(dimension(12, ndp));
  itl(dimension(ndp));
  near(dimension(ndp));
  next(dimension(ndp));
  dist(dimension(ndp));
  int iertm = fem::int0;
  int iertl = fem::int0;
  int ip1 = fem::int0;
  //C
  //C Triangulation of the data area in a plane with a scattered data
  //C point set
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine triangulates the data area in the x-y plane with
  //C a scattered data point set.  It divides the data area into a
  //C number of triangles and determines line segments that form the
  //C border of the data area.
  //C
  //C This subroutine consists of the following two steps, i.e.,
  //C (1) basic triangulation in the convex hull of the data points,
  //C and (2) removal of thin triangles along the border line of the
  //C data area.  It calls the SDTRCH and SDTRTT subroutines, that
  //C correspond to Steps (1) and (2), respectively.
  //C
  //C The input arguments are
  //C   NDP  = number of data points (must be greater than 3),
  //C   XD   = array of dimension NDP containing the x
  //C          coordinates of the data points,
  //C   YD   = array of dimension NDP containing the y
  //C          coordinates of the data points.
  //C
  //C The output arguments are
  //C   NT   = number of triangles (its maximum is 2*NDP-5),
  //C   IPT  = two-dimensional integer array of dimension
  //C          (3,NT), where the point numbers of the vertexes
  //C          of the ITth triangle are to be stored counter-
  //C          clockwise in the ITth column, where IT = 1, 2,
  //C          ..., NT,
  //C   NL   = number of border line segments (its maximum is
  //C          NDP),
  //C   IPL  = two-dimensional integer array of dimension
  //C          (2,NL), where the point numbers of the end
  //C          points of the (IL)th border line segment are to
  //C          be stored counterclockwise in the ILth column,
  //C          where IL = 1, 2, ..., NL, with the line segments
  //C          stored counterclockwise,
  //C   IERT = error flag
  //C        = 0 for no errors
  //C        = 1 for NDP = 3 or less
  //C        = 2 for identical data points
  //C        = 3 for all collinear data points.
  //C
  //C The other arguments are
  //C   LIST = integer array of dimension 6*NDP USED internally
  //C          as a work area,
  //C   LPTR = integer array of dimension 6*NDP USED internally
  //C          as a work area,
  //C   LEND = integer array of dimension NDP USED internally as
  //C          a work area,
  //C   LTRI = two-dimensional integer array of dimension 12*NDP
  //C          used internally as a work area.
  //C   ITL  = integer array of dimension NDP used internally as
  //C          a work area.
  //C
  //C agebhard@uni-klu.ac.at: added from new TRIPACK:
  //C   NEAR, NEXT, DIST work arrays from TRMESH, size NDP
  //C
  //C Specification statements
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. External Subroutines ..
  //C     ..
  //C Basic triangulation
  sdtrch(cmn, ndp, xd, yd, nt, ipt, nl, ipl, iertm, iertl, list,
    lptr, lend, ltri, near, next, dist);
  if (iertm != 0) {
    goto statement_10;
  }
  if (iertl != 0) {
    goto statement_20;
  }
  iert = 0;
  //C Removal of thin triangles that share border line segments
  sdtrtt(ndp, xd, yd, nt, ipt, nl, ipl, itl);
  return;
  //C Error exit
  statement_10:
  if (iertm ==  - 1) {
    iert = 1;
    //C     WRITE (*,FMT=9000) NDP
  }
  else if (iertm ==  - 2) {
    iert = 2;
    //C     WRITE (*,FMT=9010)
  }
  else {
    iert = 3;
    ip1 = iertm;
    //C     WRITE (*,FMT=9020) NDP,IP1,XD(IP1),YD(IP1)
  }
  return;
  statement_20:
  if (iertl == 1) {
    iert = 4;
    //C     WRITE (*,FMT=9030) NDP
  }
  else if (iertl == 2) {
    iert = 5;
    //C     WRITE (*,FMT=9040)
  }
  //C Format statements
  (void)ip1;
}

void
sdcldp(
  int const& ndp,
  arr_cref<double> xd,
  arr_cref<double> yd,
  arr_ref<int, 2> ipc,
  arr_ref<double> dsq,
  arr_ref<int> idsq)
{
  xd(dimension(ndp));
  yd(dimension(ndp));
  ipc(dimension(9, ndp));
  dsq(dimension(ndp));
  idsq(dimension(ndp));
  //C
  //C Closest data points
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine selects, at each of the data points, nine data
  //C points closest to it.
  //C
  //C The input arguments are
  //C   NDP  = number of data points,
  //C   XD   = array of dimension NDP containing the x
  //C          coordinates of the data points,
  //C   YD   = array of dimension NDP containing the y
  //C          coordinates of the data points.
  //C
  //C The output argument is
  //C   IPC  = two-dimensional integer array of dimension 9*NDP,
  //C          where the point numbers of nine data points closest
  //C          to the IDPth data point, in an ascending order of
  //C          the distance from the IDPth point, are to be
  //C          stored in the IDPth column, where IDP = 1, 2,
  //C          ..., NDP.
  //C
  //C The other arguments are
  //C   DSQ  = array of dimension NDP used as a work area,
  //C   IDSQ = integer array of dimension NDP used as a work
  //C          area.
  //C
  //C Specification statements
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C DO-loop with respect to the data point number
  int idp = fem::int0;
  int jdp = fem::int0;
  int jipcmx = fem::int0;
  int jipc = fem::int0;
  int jdsqmn = fem::int0;
  double dsqmn = fem::double0;
  int jdpmn = fem::int0;
  int idsqmn = fem::int0;
  FEM_DO_SAFE(idp, 1, ndp) {
    //C Calculates the distance squared for all data points from the
    //C IDPth data point and stores the data point number and the
    //C calculated results in the IDSQ and DSQ arrays, respectively.
    FEM_DO_SAFE(jdp, 1, ndp) {
      idsq(jdp) = jdp;
      dsq(jdp) = fem::pow2((xd(jdp) - xd(idp))) + fem::pow2((yd(
        jdp) - yd(idp)));
    }
    //C Sorts the IDSQ and DSQ arrays in such a way that the IDPth
    //C point is in the first element in each array.
    idsq(idp) = 1;
    dsq(idp) = dsq(1);
    idsq(1) = idp;
    dsq(1) = 0.0f;
    //C Selects nine data points closest to the IDPth data point and
    //C stores the data point numbers in the IPC array.
    jipcmx = fem::min(ndp - 1, 10);
    FEM_DO_SAFE(jipc, 2, jipcmx) {
      jdsqmn = jipc;
      dsqmn = dsq(jipc);
      jdpmn = jipc + 1;
      FEM_DO_SAFE(jdp, jdpmn, ndp) {
        if (dsq(jdp) < dsqmn) {
          jdsqmn = jdp;
          dsqmn = dsq(jdp);
        }
      }
      idsqmn = idsq(jdsqmn);
      idsq(jdsqmn) = idsq(jipc);
      dsq(jdsqmn) = dsq(jipc);
      idsq(jipc) = idsqmn;
    }
    FEM_DO_SAFE(jipc, 1, 9) {
      ipc(jipc, idp) = idsq(jipc + 1);
    }
  }
}

void
sdleqn(
  int const& n,
  arr_ref<double, 2> aa,
  arr_cref<double> b,
  arr_ref<double> x,
  double& det,
  double& cn,
  arr_ref<int> k,
  arr_ref<double, 2> ee,
  arr_ref<double, 2> zz)
{
  aa(dimension(n, n));
  b(dimension(n));
  x(dimension(n));
  k(dimension(n));
  ee(dimension(n, n));
  zz(dimension(n, n));
  int j = fem::int0;
  int i = fem::int0;
  int ij = fem::int0;
  double aamx = fem::double0;
  int jmx = fem::int0;
  double aaijmx = fem::double0;
  int kjmx = fem::int0;
  double aaijij = fem::double0;
  int jj = fem::int0;
  int ijp1 = fem::int0;
  double aaiij = fem::double0;
  int ijr = fem::int0;
  double sa = fem::double0;
  double sz = fem::double0;
  //C
  //C Solution of a set of linear equations
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine solves a set of linear equations.
  //C
  //C The input arguments are
  //C   N   = number of linear equations,
  //C   AA  = two-dimensional array of dimension N*N
  //C         containing the coefficients of the equations,
  //C   B   = array of dimension N containing the constant
  //C         values in the right-hand side of the equations.
  //C
  //C The output arguments are
  //C   X   = array of dimension N, where the solution is
  //C         to be stored,
  //C   DET = determinant of the AA array,
  //C   CN  = condition number of the AA matrix.
  //C
  //C The other arguments are
  //C   K   = integer array of dimension N used internally
  //C         as the work area,
  //C   EE  = two-dimensional array of dimension N*N used
  //C         internally as the work area,
  //C   ZZ  = two-dimensional array of dimension N*N used
  //C         internally as the work area.
  //C
  //C Specification statements
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C Calculation
  //C Initial setting
  FEM_DO_SAFE(j, 1, n) {
    k(j) = j;
  }
  FEM_DO_SAFE(i, 1, n) {
    FEM_DO_SAFE(j, 1, n) {
      ee(i, j) = 0.0f;
    }
    ee(i, i) = 1.0f;
  }
  //C Calculation of inverse matrix of AA
  FEM_DO_SAFE(ij, 1, n) {
    //C Finds out the element having the maximum absolute value in the
    //C IJ th row.
    aamx = fem::abs(aa(ij, ij));
    jmx = ij;
    FEM_DO_SAFE(j, ij, n) {
      if (fem::abs(aa(ij, j)) > aamx) {
        aamx = fem::abs(aa(ij, j));
        jmx = j;
      }
    }
    //C Switches two columns in such a way that the element with the
    //C maximum value is on the diagonal.
    FEM_DO_SAFE(i, 1, n) {
      aaijmx = aa(i, ij);
      aa(i, ij) = aa(i, jmx);
      aa(i, jmx) = aaijmx;
    }
    kjmx = k(ij);
    k(ij) = k(jmx);
    k(jmx) = kjmx;
    //C Makes the diagonal element to be unity.
    aaijij = aa(ij, ij);
    if (aaijij == 0.0f) {
      goto statement_210;
    }
    FEM_DO_SAFE(j, ij, n) {
      aa(ij, j) = aa(ij, j) / aaijij;
    }
    FEM_DO_SAFE(jj, 1, n) {
      ee(ij, jj) = ee(ij, jj) / aaijij;
    }
    //C Eliminates the lower left elements.
    if (ij < n) {
      ijp1 = ij + 1;
      FEM_DO_SAFE(i, ijp1, n) {
        aaiij = aa(i, ij);
        FEM_DO_SAFE(j, ijp1, n) {
          aa(i, j) = aa(i, j) - aa(ij, j) * aaiij;
        }
        FEM_DO_SAFE(jj, 1, n) {
          ee(i, jj) = ee(i, jj) - ee(ij, jj) * aaiij;
        }
      }
    }
    //C Calculates the determinant.
    if (ij == 1) {
      det = 1.0f;
    }
    det = det * aaijij * (fem::pow((-1), (ij + jmx)));
  }
  //C Calculates the elements of the inverse matrix.
  FEM_DO_SAFE(ijr, 1, n) {
    ij = n + 1 - ijr;
    if (ij < n) {
      ijp1 = ij + 1;
      FEM_DO_SAFE(j, ijp1, n) {
        FEM_DO_SAFE(jj, 1, n) {
          ee(ij, jj) = ee(ij, jj) - aa(ij, j) * ee(j, jj);
        }
      }
    }
  }
  FEM_DO_SAFE(j, 1, n) {
    i = k(j);
    FEM_DO_SAFE(jj, 1, n) {
      zz(i, jj) = ee(j, jj);
    }
  }
  //C Calculation of the condition number of AA
  sa = 0.0f;
  sz = 0.0f;
  FEM_DO_SAFE(i, 1, n) {
    FEM_DO_SAFE(j, 1, n) {
      sa += aa(i, j) * aa(j, i);
      sz += zz(i, j) * zz(j, i);
    }
  }
  cn = fem::sqrt(fem::abs(sa * sz));
  //C Calculation of X vector
  FEM_DO_SAFE(i, 1, n) {
    x(i) = 0.0f;
    FEM_DO_SAFE(j, 1, n) {
      x(i) += zz(i, j) * b(j);
    }
  }
  return;
  //C Special case where the determinant is zero
  statement_210:
  FEM_DO_SAFE(i, 1, n) {
    x(i) = 0.0f;
  }
  det = 0.0f;
}

void
sdcf3p(
  int const& ndp,
  arr_cref<double> xd,
  arr_cref<double> yd,
  arr_cref<double> zd,
  arr_cref<int, 2> ipc,
  arr_ref<double, 2> cf,
  arr_ref<int> ncp)
{
  xd(dimension(ndp));
  yd(dimension(ndp));
  zd(dimension(ndp));
  ipc(dimension(9, ndp));
  cf(dimension(9, ndp));
  ncp(dimension(ndp));
  int idp = fem::int0;
  int j = fem::int0;
  int i = fem::int0;
  const int n3 = 10;
  int idpi = fem::int0;
  double x = fem::double0;
  double y = fem::double0;
  arr_2d<n3, n3, double> aa3(fem::fill0);
  arr_1d<n3, double> b(fem::fill0);
  arr_1d<n3, double> cfi(fem::fill0);
  double det = fem::double0;
  double cn = fem::double0;
  arr_1d<n3, int> k(fem::fill0);
  arr_2d<n3, n3, double> ee(fem::fill0);
  arr_2d<n3, n3, double> zz(fem::fill0);
  const double cnrmx = 1.5e+04f;
  const int n2 = 6;
  arr_2d<n2, n2, double> aa2(fem::fill0);
  const int n1 = 3;
  arr_2d<n1, n1, double> aa1(fem::fill0);
  double x1 = fem::double0;
  double y1 = fem::double0;
  double z1 = fem::double0;
  double x2 = fem::double0;
  double y2 = fem::double0;
  double z2 = fem::double0;
  //C
  //C Coefficients of the third-degree polynomial for z(x,y)
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine calculates, for each data point, coefficients
  //C of the third-degree polynomial for z(x,y) fitted to the set of
  //C 10 data points consisting of the data point in question and
  //C nine data points closest to it.  When the condition number of
  //C the matrix associated with the 10 data point set is too large,
  //C this subroutine calculates coefficients of the second-degree
  //C polynomial fitted to the set of six data points consisting of
  //C the data point in question and five data points closest to it.
  //C When the condition number of the matrix associated with the six
  //C data point set is too large, this subroutine calculates
  //C coefficients of the first-degree polynomial fitted to the set of
  //C three data points closest to the data point in question.  When
  //C the condition number of the matrix associated with the three data
  //C point set is too large, this subroutine calculates coefficients
  //C of the first-degree polynomial fitted to the set of two data
  //C points consisting of the data point in question and one data
  //C point closest to it, assuming that the plane represented by the
  //C polynomial is horizontal in the direction which is at right
  //C angles to the line connecting the two data points.
  //C
  //C The input arguments are
  //C   NDP = number of data points,
  //C   XD  = array of dimension NDP containing the x
  //C         coordinates of the data points,
  //C   YD  = array of dimension NDP containing the y
  //C         coordinates of the data points,
  //C   ZD  = array of dimension NDP containing the z values
  //C         at the data points,
  //C   IPC = two-dimensional integer array of dimension
  //C         9*NDP containing the point numbers of 9 data
  //C         points closest to the IDPth data point in the
  //C         IDPth column, where IDP = 1, 2, ..., NDP.
  //C
  //C The output arguments are
  //C   CF  = two-dimensional array of dimension 9*NDP,
  //C         where the coefficients of the polynomial
  //C         (a10, a20, a30, a01, a11, a21, a02, a12, a03)
  //C         calculated at the IDPth data point are to be
  //C         stored in the IDPth column, where IDP = 1, 2,
  //C         ..., NDP,
  //C   NCP = integer array of dimension NDP, where the numbers
  //C         of the closest points used are to be stored.
  //C
  //C The constant in the first PARAMETER statement below is
  //C   CNRMX = maximum value of the ratio of the condition
  //C           number of the matrix associated with the point
  //C           set to the number of points.
  //C The constant value has been selected empirically.
  //C
  //C The N1, N2, and N3 constants in the second PARAMETER statement
  //C are the numbers of the data points used to determine the first-,
  //C second-, and third-degree polynomials, respectively.
  //C
  //C This subroutine calls the SDLEQN subroutine.
  //C
  //C Specification statements
  //C     .. Parameters ..
  //C     ..
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Local Arrays ..
  //C     ..
  //C     .. External Subroutines ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C Main DO-loop with respect to the data point
  FEM_DO_SAFE(idp, 1, ndp) {
    FEM_DO_SAFE(j, 1, 9) {
      cf(j, idp) = 0.0f;
    }
    //C Calculates the coefficients of the set of linear equations
    //C with the 10-point data point set.
    FEM_DO_SAFE(i, 1, n3) {
      if (i == 1) {
        idpi = idp;
      }
      else {
        idpi = ipc(i - 1, idp);
      }
      x = xd(idpi);
      y = yd(idpi);
      aa3(i, 1) = 1.0f;
      aa3(i, 2) = x;
      aa3(i, 3) = x * x;
      aa3(i, 4) = x * x * x;
      aa3(i, 5) = y;
      aa3(i, 6) = x * y;
      aa3(i, 7) = x * x * y;
      aa3(i, 8) = y * y;
      aa3(i, 9) = x * y * y;
      aa3(i, 10) = y * y * y;
      b(i) = zd(idpi);
    }
    //C Solves the set of linear equations.
    sdleqn(n3, aa3, b, cfi, det, cn, k, ee, zz);
    //C Stores the calculated results as the coefficients of the
    //C third-degree polynomial when applicable.
    if (det != 0.0f) {
      if (cn <= cnrmx * fem::dble(n3)) {
        FEM_DO_SAFE(j, 2, n3) {
          cf(j - 1, idp) = cfi(j);
        }
        ncp(idp) = n3 - 1;
        goto statement_60;
      }
    }
    //C Calculates the coefficients of the set of linear equations
    //C with the 6-point data point set.
    FEM_DO_SAFE(i, 1, n2) {
      if (i == 1) {
        idpi = idp;
      }
      else {
        idpi = ipc(i - 1, idp);
      }
      x = xd(idpi);
      y = yd(idpi);
      aa2(i, 1) = 1.0f;
      aa2(i, 2) = x;
      aa2(i, 3) = x * x;
      aa2(i, 4) = y;
      aa2(i, 5) = x * y;
      aa2(i, 6) = y * y;
      b(i) = zd(idpi);
    }
    //C Solves the set of linear equations.
    sdleqn(n2, aa2, b, cfi, det, cn, k, ee, zz);
    //C Stores the calculated results as the coefficients of the
    //C second-degree polynomial when applicable.
    if (det != 0.0f) {
      if (cn <= cnrmx * fem::dble(n2)) {
        cf(1, idp) = cfi(2);
        cf(2, idp) = cfi(3);
        cf(4, idp) = cfi(4);
        cf(5, idp) = cfi(5);
        cf(7, idp) = cfi(6);
        ncp(idp) = n2 - 1;
        goto statement_60;
      }
    }
    //C Calculates the coefficients of the set of linear equations
    //C with the 3-point data point set.
    FEM_DO_SAFE(i, 1, n1) {
      idpi = ipc(i, idp);
      x = xd(idpi);
      y = yd(idpi);
      aa1(i, 1) = 1.0f;
      aa1(i, 2) = x;
      aa1(i, 3) = y;
      b(i) = zd(idpi);
    }
    //C Solves the set of linear equations.
    sdleqn(n1, aa1, b, cfi, det, cn, k, ee, zz);
    //C Stores the calculated results as the coefficients of the
    //C first-degree polynomial when applicable.
    if (det != 0.0f) {
      if (cn <= cnrmx * fem::dble(n1)) {
        cf(1, idp) = cfi(2);
        cf(4, idp) = cfi(3);
        ncp(idp) = n1;
        goto statement_60;
      }
    }
    //C Calculates the coefficients of the set of linear equations
    //C with the 2-point data point set when applicable.
    idpi = idp;
    x1 = xd(idpi);
    y1 = yd(idpi);
    z1 = zd(idpi);
    idpi = ipc(1, idp);
    x2 = xd(idpi);
    y2 = yd(idpi);
    z2 = zd(idpi);
    cf(1, idp) = (x2 - x1) * (z2 - z1) / (fem::pow2((x2 - x1)) +
      fem::pow2((y2 - y1)));
    cf(4, idp) = (y2 - y1) * (z2 - z1) / (fem::pow2((x2 - x1)) +
      fem::pow2((y2 - y1)));
    ncp(idp) = 1;
    statement_60:;
  }
}

void
sdls1p(
  int const& ndp,
  arr_cref<double> xd,
  arr_cref<double> yd,
  arr_cref<double> zd,
  arr_cref<int, 2> ipc,
  arr_cref<int> ncp,
  arr_ref<double, 2> cfl1)
{
  xd(dimension(ndp));
  yd(dimension(ndp));
  zd(dimension(ndp));
  ipc(dimension(9, ndp));
  ncp(dimension(ndp));
  cfl1(dimension(2, ndp));
  int idp = fem::int0;
  int npls = fem::int0;
  double sx = fem::double0;
  double sy = fem::double0;
  double sxx = fem::double0;
  double sxy = fem::double0;
  double syy = fem::double0;
  double sz = fem::double0;
  double sxz = fem::double0;
  double syz = fem::double0;
  int i = fem::int0;
  int idpi = fem::int0;
  double x = fem::double0;
  double y = fem::double0;
  double z = fem::double0;
  double an = fem::double0;
  double a11 = fem::double0;
  double a12 = fem::double0;
  double a22 = fem::double0;
  double b1 = fem::double0;
  double b2 = fem::double0;
  double dlt = fem::double0;
  double x1 = fem::double0;
  double y1 = fem::double0;
  double z1 = fem::double0;
  double x2 = fem::double0;
  double y2 = fem::double0;
  double z2 = fem::double0;
  //C
  //C Least squares fit of a linear surface (plane) to z(x,y) values
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine performs the least squares fit of a linear
  //C surface (plane) to a data point set consisting of the data
  //C point in question and several data points closest to it used
  //C in the SDCF3P subroutine.
  //C
  //C The input arguments are
  //C   NDP  = number of data points,
  //C   XD   = array of dimension NDP containing the x coordinates
  //C          of the data points,
  //C   YD   = array of dimension NDP containing the y coordinates
  //C          of the data points,
  //C   ZD   = array of dimension NDP containing the z values at
  //C          the data points,
  //C   IPC  = two-dimensional integer array of dimension 9*NDP
  //C          containing, in the IDPth column, point numbers of
  //C          nine data points closest to the IDPth data point,
  //C          where IDP = 1, 2, ..., NDP,
  //C   NCP  = integer array of dimension NDP containing the
  //C          numbers of the closest points used in the SDCF3P
  //C          subroutine.
  //C
  //C The output argument is
  //C   CFL1 = two-dimensional array of dimension 2*NDP, where
  //C          the coefficients (a10, a01) of the least squares
  //C          fit, first-degree polynomial calculated at the
  //C          IDPth data point are to be stored in the IDPth
  //C          column, where IDP = 1, 2, ..., NDP.
  //C
  //C Before this subroutine is called, the SDCF3P subroutine must
  //C have been called.
  //C
  //C Specification statements
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C DO-loop with respect to the data point
  FEM_DO_SAFE(idp, 1, ndp) {
    npls = ncp(idp) + 1;
    if (npls == 2) {
      goto statement_20;
    }
    //C Performs the least squares fit of a plane.
    sx = 0.0f;
    sy = 0.0f;
    sxx = 0.0f;
    sxy = 0.0f;
    syy = 0.0f;
    sz = 0.0f;
    sxz = 0.0f;
    syz = 0.0f;
    FEM_DO_SAFE(i, 1, npls) {
      if (i == 1) {
        idpi = idp;
      }
      else {
        idpi = ipc(i - 1, idp);
      }
      x = xd(idpi);
      y = yd(idpi);
      z = zd(idpi);
      sx += x;
      sy += y;
      sxx += x * x;
      sxy += x * y;
      syy += y * y;
      sz += z;
      sxz += x * z;
      syz += y * z;
    }
    an = npls;
    a11 = an * sxx - sx * sx;
    a12 = an * sxy - sx * sy;
    a22 = an * syy - sy * sy;
    b1 = an * sxz - sx * sz;
    b2 = an * syz - sy * sz;
    dlt = a11 * a22 - a12 * a12;
    cfl1(1, idp) = (b1 * a22 - b2 * a12) / dlt;
    cfl1(2, idp) = (b2 * a11 - b1 * a12) / dlt;
    goto statement_30;
    statement_20:
    idpi = idp;
    x1 = xd(idpi);
    y1 = yd(idpi);
    z1 = zd(idpi);
    idpi = ipc(1, idp);
    x2 = xd(idpi);
    y2 = yd(idpi);
    z2 = zd(idpi);
    cfl1(1, idp) = (x2 - x1) * (z2 - z1) / (fem::pow2((x2 - x1)) +
      fem::pow2((y2 - y1)));
    cfl1(2, idp) = (y2 - y1) * (z2 - z1) / (fem::pow2((x2 - x1)) +
      fem::pow2((y2 - y1)));
    statement_30:;
  }
}

void
sdpd3p(
  int const& ndp,
  arr_cref<double> xd,
  arr_cref<double> yd,
  arr_cref<double> zd,
  arr_ref<double, 2> pdd,
  arr_ref<double, 2> cf3,
  arr_ref<double, 2> cfl1,
  arr_ref<double> dsq,
  arr_ref<int> idsq,
  arr_ref<int, 2> ipc,
  arr_ref<int> ncp)
{
  xd(dimension(ndp));
  yd(dimension(ndp));
  zd(dimension(ndp));
  pdd(dimension(5, ndp));
  cf3(dimension(9, ndp));
  cfl1(dimension(2, ndp));
  dsq(dimension(ndp));
  idsq(dimension(ndp));
  ipc(dimension(9, ndp));
  ncp(dimension(ndp));
  int idp1 = fem::int0;
  int npe = fem::int0;
  int idp2 = fem::int0;
  int ncp2 = fem::int0;
  int ncp2p1 = fem::int0;
  int j = fem::int0;
  const int npemx = 25;
  arr_2d<10, npemx, int> ipcpe(fem::fill0);
  int j1 = fem::int0;
  int jmn = fem::int0;
  int imn = fem::int0;
  int j2 = fem::int0;
  int ipe1 = fem::int0;
  arr_1d<npemx, int> idppe(fem::fill0);
  int idppe1 = fem::int0;
  const int npeamn = 3;
  int jj = fem::int0;
  const int npeamx = 6;
  double x = fem::double0;
  double y = fem::double0;
  int ipe = fem::int0;
  int idpi = fem::int0;
  double a10 = fem::double0;
  double a20 = fem::double0;
  double a30 = fem::double0;
  double a01 = fem::double0;
  double a11 = fem::double0;
  double a21 = fem::double0;
  double a02 = fem::double0;
  double a12 = fem::double0;
  double a03 = fem::double0;
  arr_2d<5, npemx, double> pdpe(fem::fill0);
  double anpe = fem::double0;
  double anpem1 = fem::double0;
  int k = fem::int0;
  arr_1d<5, double> ampdpe(fem::fill0);
  arr_1d<5, double> sspdpe(fem::fill0);
  double alpwt = fem::double0;
  arr_1d<npemx, double> pwt(fem::fill0);
  double zx = fem::double0;
  double zy = fem::double0;
  arr_1d<npemx, double> rvwt(fem::fill0);
  arr_1d<5, double> pddif(fem::fill0);
  arr_1d<5, double> pddii(fem::fill0);
  double smwtf = fem::double0;
  double smwti = fem::double0;
  double wtf = fem::double0;
  double wti = fem::double0;
  //C
  //C Partial derivatives for bivariate interpolation and surface
  //C fitting for scattered data
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine estimates partial derivatives of the first and
  //C second orders at the data points for bivariate interpolation
  //C and surface fitting for scattered data.  In most cases, this
  //C subroutine has the accuracy of a cubic (third-degree)
  //C polynomial.
  //C
  //C The input arguments are
  //C   NDP  = number of data points,
  //C   XD   = array of dimension NDP containing the x
  //C          coordinates of the data points,
  //C   YD   = array of dimension NDP containing the y
  //C          coordinates of the data points,
  //C   ZD   = array of dimension NDP containing the z values
  //C          at the data points.
  //C
  //C The output argument is
  //C   PDD  = two-dimensional array of dimension 5*NDP, where
  //C          the estimated zx, zy, zxx, zxy, and zyy values
  //C          at the IDPth data point are to be stored in the
  //C          IDPth row, where IDP = 1, 2, ..., NDP.
  //C
  //C The other arguments are
  //C   CF3  = two-dimensional array of dimension 9*NDP used
  //C          internally as a work area,
  //C   CFL1 = two-dimensional array of dimension 2*NDP used
  //C          internally as a work area,
  //C   DSQ  = array of dimension NDP used internally as a work
  //C          area,
  //C   IDSQ = integer array of dimension NDP used internally
  //C          as a work area,
  //C   IPC  = two-dimensional integer array of dimension 9*NDP
  //C          used internally as a work area,
  //C   NCP  = integer array of dimension NDP used internally
  //C          as a work area.
  //C
  //C The constant in the first PARAMETER statement below is
  //C   NPEMX = maximum number of primary estimates.
  //C The constant value has been selected empirically.
  //C
  //C The constants in the second PARAMETER statement below are
  //C   NPEAMN = minimum number of primary estimates,
  //C   NPEAMX = maximum number of primary estimates when
  //C            additional primary estimates are added.
  //C The constant values have been selected empirically.
  //C
  //C This subroutine calls the SDCLDP, SDCF3P, and SDLS1P
  //C subroutines.
  //C
  //C Specification statements
  //C     .. Parameters ..
  //C     ..
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Local Arrays ..
  //C     ..
  //C     .. External Subroutines ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C Calculation
  //C Selects, at each of the data points, nine data points closest
  //C to the data point in question.
  sdcldp(ndp, xd, yd, ipc, dsq, idsq);
  //C Fits, at each of the data points, a cubic (third-degree)
  //C polynomial to z values at the 10 data points that consist of
  //C the data point in question and 9 data points closest to it.
  sdcf3p(ndp, xd, yd, zd, ipc, cf3, ncp);
  //C Performs, at each of the data points, the least-squares fit of
  //C a plane to z values at the 10 data points.
  sdls1p(ndp, xd, yd, zd, ipc, ncp, cfl1);
  //C Outermost DO-loop with respect to the data point
  FEM_DO_SAFE(idp1, 1, ndp) {
    //C Selects data point sets for sets of primary estimates of partial
    //C derivatives.
    //C - Selects a candidate.
    npe = 0;
    FEM_DO_SAFE(idp2, 1, ndp) {
      ncp2 = ncp(idp2);
      ncp2p1 = ncp2 + 1;
      if (idp2 == idp1) {
        goto statement_20;
      }
      FEM_DO_SAFE(j, 1, ncp2) {
        if (ipc(j, idp2) == idp1) {
          goto statement_20;
        }
      }
      goto statement_80;
      statement_20:
      ipcpe(1, npe + 1) = idp2;
      FEM_DO_SAFE(j, 1, ncp2) {
        ipcpe(j + 1, npe + 1) = ipc(j, idp2);
      }
      FEM_DO_SAFE(j1, 1, ncp2) {
        jmn = j1;
        imn = ipcpe(jmn, npe + 1);
        FEM_DO_SAFE(j2, j1, ncp2p1) {
          if (ipcpe(j2, npe + 1) < imn) {
            jmn = j2;
            imn = ipcpe(jmn, npe + 1);
          }
        }
        ipcpe(jmn, npe + 1) = ipcpe(j1, npe + 1);
        ipcpe(j1, npe + 1) = imn;
      }
      //C - Checks whether or not the candidate has already been included.
      if (npe > 0) {
        FEM_DO_SAFE(ipe1, 1, npe) {
          idppe1 = idppe(ipe1);
          if (ncp2 != ncp(idppe1)) {
            goto statement_70;
          }
          FEM_DO_SAFE(j, 1, ncp2p1) {
            if (ipcpe(j, npe + 1) != ipcpe(j, ipe1)) {
              goto statement_70;
            }
          }
          goto statement_80;
          statement_70:;
        }
      }
      npe++;
      idppe(npe) = idp2;
      if (npe >= npemx) {
        goto statement_90;
      }
      statement_80:;
    }
    statement_90:
    //C Adds additional closest data points when necessary.
    if (npe < npeamn) {
      FEM_DO_SAFE(jj, 1, 9) {
        idp2 = ipc(jj, idp1);
        ncp2 = ncp(idp2);
        ncp2p1 = ncp2 + 1;
        ipcpe(1, npe + 1) = idp2;
        FEM_DO_SAFE(j, 1, ncp2) {
          ipcpe(j + 1, npe + 1) = ipc(j, idp2);
        }
        FEM_DO_SAFE(j1, 1, ncp2) {
          jmn = j1;
          imn = ipcpe(jmn, npe + 1);
          FEM_DO_SAFE(j2, j1, ncp2p1) {
            if (ipcpe(j2, npe + 1) < imn) {
              jmn = j2;
              imn = ipcpe(jmn, npe + 1);
            }
          }
          ipcpe(jmn, npe + 1) = ipcpe(j1, npe + 1);
          ipcpe(j1, npe + 1) = imn;
        }
        if (npe > 0) {
          FEM_DO_SAFE(ipe1, 1, npe) {
            idppe1 = idppe(ipe1);
            if (ncp2 != ncp(idppe1)) {
              goto statement_140;
            }
            FEM_DO_SAFE(j, 1, ncp2p1) {
              if (ipcpe(j, npe + 1) != ipcpe(j, ipe1)) {
                goto statement_140;
              }
            }
            goto statement_150;
            statement_140:;
          }
        }
        npe++;
        idppe(npe) = idp2;
        if (npe >= npeamx) {
          goto statement_160;
        }
        statement_150:;
      }
    }
    statement_160:
    //C Calculates the primary estimates of partial derivatives.
    x = xd(idp1);
    y = yd(idp1);
    FEM_DO_SAFE(ipe, 1, npe) {
      idpi = idppe(ipe);
      a10 = cf3(1, idpi);
      a20 = cf3(2, idpi);
      a30 = cf3(3, idpi);
      a01 = cf3(4, idpi);
      a11 = cf3(5, idpi);
      a21 = cf3(6, idpi);
      a02 = cf3(7, idpi);
      a12 = cf3(8, idpi);
      a03 = cf3(9, idpi);
      pdpe(1, ipe) = a10 + x * (2.0f * a20 + x * 3.0f * a30) + y * (
        a11 + 2.0f * a21 * x + a12 * y);
      pdpe(2, ipe) = a01 + y * (2.0f * a02 + y * 3.0f * a03) + x * (
        a11 + 2.0f * a12 * y + a21 * x);
      pdpe(3, ipe) = 2.0f * a20 + 6.0f * a30 * x + 2.0f * a21 * y;
      pdpe(4, ipe) = a11 + 2.0f * a21 * x + 2.0f * a12 * y;
      pdpe(5, ipe) = 2.0f * a02 + 6.0f * a03 * y + 2.0f * a12 * x;
    }
    if (npe == 1) {
      goto statement_290;
    }
    //C Weighted values of partial derivatives (through the statement
    //C labeled 280 + 1)
    //C Calculates the probability weight.
    anpe = fem::dble(npe);
    anpem1 = fem::dble(npe - 1);
    FEM_DO_SAFE(k, 1, 5) {
      ampdpe(k) = 0.0f;
      sspdpe(k) = 0.0f;
      FEM_DO_SAFE(ipe, 1, npe) {
        ampdpe(k) += pdpe(k, ipe);
        sspdpe(k) += fem::pow2(pdpe(k, ipe));
      }
      ampdpe(k) = ampdpe(k) / anpe;
      sspdpe(k) = (sspdpe(k) - anpe * fem::pow2(ampdpe(k))) / anpem1;
    }
    FEM_DO_SAFE(ipe, 1, npe) {
      alpwt = 0.0f;
      FEM_DO_SAFE(k, 1, 5) {
        if (sspdpe(k) != 0.0f) {
          alpwt += (fem::pow2((pdpe(k, ipe) - ampdpe(k)))) / sspdpe(k);
        }
      }
      pwt(ipe) = fem::exp(-alpwt / 2.0f);
    }
    //C Calculates the reciprocal of the volatility weight.
    FEM_DO_SAFE(ipe, 1, npe) {
      idpi = idppe(ipe);
      zx = cfl1(1, idpi);
      zy = cfl1(2, idpi);
      rvwt(ipe) = (fem::pow2((pdpe(1, ipe) - zx)) + fem::pow2((pdpe(2,
        ipe) - zy))) * (fem::pow2(pdpe(3, ipe)) + 2.0f * fem::pow2(pdpe(4,
        ipe)) + fem::pow2(pdpe(5, ipe)));
      //C             ZXX=0.0
      //C             ZXY=0.0
      //C             ZYY=0.0
      //C             RVWT(IPE)=((PDPE(1,IPE)-ZX)**2+(PDPE(2,IPE)-ZY)**2)
      //C    +                 *((PDPE(3,IPE)-ZXX)**2+2.0*(PDPE(4,IPE)-ZXY)**2
      //C    +                  +(PDPE(5,IPE)-ZYY)**2)
    }
    //C Calculates the weighted values of partial derivatives.
    FEM_DO_SAFE(k, 1, 5) {
      pddif(k) = 0.0f;
      pddii(k) = 0.0f;
    }
    smwtf = 0.0f;
    smwti = 0.0f;
    FEM_DO_SAFE(ipe, 1, npe) {
      if (rvwt(ipe) > 0.0f) {
        wtf = pwt(ipe) / rvwt(ipe);
        FEM_DO_SAFE(k, 1, 5) {
          pddif(k) += pdpe(k, ipe) * wtf;
        }
        smwtf += wtf;
      }
      else {
        wti = pwt(ipe);
        FEM_DO_SAFE(k, 1, 5) {
          pddii(k) += pdpe(k, ipe) * wti;
        }
        smwti += wti;
      }
    }
    if (smwti <= 0.0f) {
      FEM_DO_SAFE(k, 1, 5) {
        pdd(k, idp1) = pddif(k) / smwtf;
      }
    }
    else {
      FEM_DO_SAFE(k, 1, 5) {
        pdd(k, idp1) = pddii(k) / smwti;
      }
    }
    goto statement_310;
    //C Only one qualified point set
    statement_290:
    FEM_DO_SAFE(k, 1, 5) {
      pdd(k, idp1) = pdpe(k, 1);
    }
    statement_310:;
  }
}

void
sdlctn(
  int const& ndp,
  arr_cref<double> xd,
  arr_cref<double> yd,
  int const& nt,
  arr_cref<int, 2> ipt,
  int const& nl,
  arr_cref<int, 2> ipl,
  int const& nip,
  arr_cref<double> xi,
  arr_cref<double> yi,
  arr_ref<int> ktli,
  arr_ref<int> itli)
{
  xd(dimension(ndp));
  yd(dimension(ndp));
  ipt(dimension(3, nt));
  ipl(dimension(2, nl));
  xi(dimension(nip));
  yi(dimension(nip));
  ktli(dimension(nip));
  itli(dimension(nip));
  // double u1 = fem::double0;
  // double v1 = fem::double0;
  // double u2 = fem::double0;
  // double v2 = fem::double0;
  // double u3 = fem::double0;
  // double v3 = fem::double0;
  int iip = fem::int0;
  double x0 = fem::double0;
  double y0 = fem::double0;
  int ktlipv = fem::int0;
  int itlipv = fem::int0;
  int itii = fem::int0;
  int ip1 = fem::int0;
  int ip2 = fem::int0;
  int ip3 = fem::int0;
  double x1 = fem::double0;
  double y1 = fem::double0;
  double x2 = fem::double0;
  double y2 = fem::double0;
  double x3 = fem::double0;
  double y3 = fem::double0;
  int ilii = fem::int0;
  int il1 = fem::int0;
  int il2 = fem::int0;
  //C
  //C Locating points in a scattered data point set
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine locates points in a scattered data point set in
  //C the x-y plane, i.e., determines to which triangle each of the
  //C points to be located belongs.  When a point to be located does
  //C not lie inside the data area, this subroutine determines the
  //C border line segment when the point lies in an outside rectangle,
  //C in an outside triangle, or in the overlap of two outside
  //C rectangles.
  //C
  //C The input arguments are
  //C   NDP  = number of data points,
  //C   XD   = array of dimension NDP containing the x
  //C          coordinates of the data points,
  //C   YD   = array of dimension NDP containing the y
  //C          coordinates of the data points,
  //C   NT   = number of triangles,
  //C   IPT  = two-dimensional integer array of dimension 3*NT
  //C          containing the point numbers of the vertexes of
  //C          the triangles,
  //C   NL   = number of border line segments,
  //C   IPL  = two-dimensional integer array of dimension 2*NL
  //C          containing the point numbers of the end points of
  //C          the border line segments,
  //C   NIP  = number of points to be located,
  //C   XI   = array of dimension NIP containing the x
  //C          coordinates of the points to be located,
  //C   YI   = array of dimension NIP containing the y
  //C          coordinates of the points to be located.
  //C
  //C The output arguments are
  //C   KTLI = integer array of dimension NIP, where the code
  //C          for the type of the piece of plane in which each
  //C          interpolated point lies is to be stored
  //C        = 1 for a triangle inside the data area
  //C        = 2 for a rectangle on the right-hand side of a
  //C            border line segment
  //C        = 3 for a triangle between two rectangles on the
  //C            right-hand side of two consecutive border line
  //C            segments
  //C        = 4 for a triangle which is an overlap of two
  //C            rectangles on the right-hand side of two
  //C            consecutive border line segments,
  //C   ITLI = integer array of dimension NIP, where the
  //C          triangle numbers or the (second) border line
  //C          segment numbers corresponding to the points to
  //C          be located are to be stored.
  //C
  //C Specification statements
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C     .. Statement Functions ..
  //C     ..
  //C Statement Function definitions
  #define spdt(u1, v1, u2, v2, u3, v3) (u1 - u3) * (u2 - u3) + (v1 - v3) * (v2 - v3)
  #define vpdt(u1, v1, u2, v2, u3, v3) (u1 - u3) * (v2 - v3) - (v1 - v3) * (u2 - u3)
  //C     ..
  //C Outermost DO-loop with respect to the points to be located
  FEM_DO_SAFE(iip, 1, nip) {
    x0 = xi(iip);
    y0 = yi(iip);
    if (iip == 1) {
      ktlipv = 0;
      itlipv = 0;
    }
    else {
      ktlipv = ktli(iip - 1);
      itlipv = itli(iip - 1);
    }
    //C Checks if in the same inside triangle as previous.
    if (ktlipv == 1) {
      itii = itlipv;
      ip1 = ipt(1, itii);
      ip2 = ipt(2, itii);
      ip3 = ipt(3, itii);
      x1 = xd(ip1);
      y1 = yd(ip1);
      x2 = xd(ip2);
      y2 = yd(ip2);
      x3 = xd(ip3);
      y3 = yd(ip3);
      if ((vpdt(x1, y1, x2, y2, x0, y0) >= 0.0f) && (vpdt(x2, y2, x3,
          y3, x0, y0) >= 0.0f) && (vpdt(x3, y3, x1, y1, x0,
          y0) >= 0.0f)) {
        ktli(iip) = 1;
        itli(iip) = itii;
        goto statement_40;
      }
    }
    //C Locates inside the data area.
    FEM_DO_SAFE(itii, 1, nt) {
      ip1 = ipt(1, itii);
      ip2 = ipt(2, itii);
      ip3 = ipt(3, itii);
      x1 = xd(ip1);
      y1 = yd(ip1);
      x2 = xd(ip2);
      y2 = yd(ip2);
      x3 = xd(ip3);
      y3 = yd(ip3);
      if ((vpdt(x1, y1, x2, y2, x0, y0) >= 0.0f) && (vpdt(x2, y2, x3,
          y3, x0, y0) >= 0.0f) && (vpdt(x3, y3, x1, y1, x0,
          y0) >= 0.0f)) {
        ktli(iip) = 1;
        itli(iip) = itii;
        goto statement_40;
      }
    }
    //C Locates outside the data area.
    FEM_DO_SAFE(ilii, 1, nl) {
      il1 = ilii;
      il2 = fem::mod(il1, nl) + 1;
      ip1 = ipl(1, il1);
      ip2 = ipl(1, il2);
      ip3 = ipl(2, il2);
      x1 = xd(ip1);
      y1 = yd(ip1);
      x2 = xd(ip2);
      y2 = yd(ip2);
      x3 = xd(ip3);
      y3 = yd(ip3);
      if (vpdt(x1, y1, x3, y3, x0, y0) <= 0.0f) {
        if (vpdt(x1, y1, x3, y3, x2, y2) <= 0.0f) {
          if ((spdt(x1, y1, x0, y0, x2, y2) <= 0.0f) && (spdt(x3, y3,
              x0, y0, x2, y2) <= 0.0f)) {
            ktli(iip) = 3;
            itli(iip) = il2;
            goto statement_40;
          }
        }
        if (vpdt(x1, y1, x3, y3, x2, y2) >= 0.0f) {
          if ((spdt(x1, y1, x0, y0, x2, y2) >= 0.0f) && (spdt(x3, y3,
              x0, y0, x2, y2) >= 0.0f)) {
            ktli(iip) = 4;
            itli(iip) = il2;
            goto statement_40;
          }
        }
      }
    }
    FEM_DO_SAFE(ilii, 1, nl) {
      il2 = ilii;
      ip2 = ipl(1, il2);
      ip3 = ipl(2, il2);
      x2 = xd(ip2);
      y2 = yd(ip2);
      x3 = xd(ip3);
      y3 = yd(ip3);
      if (vpdt(x2, y2, x3, y3, x0, y0) <= 0.0f) {
        if ((spdt(x3, y3, x0, y0, x2, y2) >= 0.0f) && (spdt(x2, y2,
            x0, y0, x3, y3) >= 0.0f)) {
          ktli(iip) = 2;
          itli(iip) = il2;
          goto statement_40;
        }
      }
    }
    statement_40:;
  }
  #undef spdt
  #undef vpdt
}

void
sdplnl(
  int const& ndp,
  arr_cref<double> xd,
  arr_cref<double> yd,
  arr_cref<double> zd,
  int const& nt,
  arr_cref<int, 2> ipt,
  int const& nl,
  arr_cref<int, 2> ipl,
  arr_cref<double, 2> pdd,
  int const& nip,
  arr_cref<double> xi,
  arr_cref<double> yi,
  arr_cref<int> ktli,
  arr_cref<int> itli,
  arr_ref<double> zi,
  arr_ref<bool> extrpi)
{
  xd(dimension(ndp));
  yd(dimension(ndp));
  zd(dimension(ndp));
  ipt(dimension(3, nt));
  ipl(dimension(2, nl));
  pdd(dimension(5, ndp));
  xi(dimension(nip));
  yi(dimension(nip));
  ktli(dimension(nip));
  itli(dimension(nip));
  zi(dimension(nip));
  extrpi(dimension(nip));
  //C
  //C Polynomials
  //C (a supporting subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine determines a polynomial in x and y for each
  //C triangle or rectangle in the x-y plane and calculates the z
  //C value by evaluating the polynomial for the desired points,
  //C for bivariate interpolation and surface fitting for scattered
  //C data.
  //C
  //C The input arguments are
  //C   NDP  = number of data points,
  //C   XD   = array of dimension NDP containing the x
  //C          coordinates of the data points,
  //C   YD   = array of dimension NDP containing the y
  //C          coordinates of the data points,
  //C   ZD   = array of dimension NDP containing the z
  //C          values at the data points,
  //C   NT   = number of triangles,
  //C   IPT  = two-dimensional integer array of dimension 3*NT
  //C          containing the point numbers of the vertexes of
  //C          the triangles,
  //C   NL   = number of border line segments,
  //C   IPL  = two-dimensional integer array of dimension 2*NL
  //C          containing the point numbers of the end points of
  //C          the border line segments,
  //C   PDD  = two-dimensional array of dimension 5*NDP
  //C          containing the partial derivatives at the data
  //C          points,
  //C   NIP  = number of output points at which interpolation is
  //C          to be performed,
  //C   XI   = array of dimension NIP containing the x
  //C          coordinates of the output points,
  //C   YI   = array of dimension NIP containing the y
  //C          coordinates of the output points,
  //C   KTLI = integer array of dimension NIP, each element
  //C          containing the code for the type of the piece of
  //C          the plane in which each output point lies
  //C        = 1 for a triangle inside the data area
  //C        = 2 for a rectangle on the right-hand side of a
  //C            border line segment
  //C        = 3 for a triangle between two rectangles on the
  //C            right-hand side of two consecutive border
  //C            line segments
  //C        = 4 for the triangle which is an overlap of two
  //C            rectangles on the right-hand side of two
  //C            consecutive border line segments,
  //C   ITLI = integer array of dimension NIP containing the
  //C          triangle numbers or the (second) border line
  //C          segment numbers corresponding to the output
  //C          points.
  //C
  //C The output argument is
  //C   ZI   = array of dimension NIP, where the calculated z
  //C          values are to be stored.
  //C   EXTRPI = logical array of dimension NIP, indicating
  //C            if a point resides outside the convex hull (and its Z value
  //C            has been extrapolated)
  //C
  //C Specification statements
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Local Arrays ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C Outermost DO-loop with respect to the output point
  int iip = fem::int0;
  double xii = fem::double0;
  double yii = fem::double0;
  int ktlii = fem::int0;
  int itlii = fem::int0;
  int ktlipv = fem::int0;
  int itlipv = fem::int0;
  int i = fem::int0;
  int idp = fem::int0;
  arr_1d<3, double> x(fem::fill0);
  arr_1d<3, double> y(fem::fill0);
  arr_1d<3, double> z(fem::fill0);
  int k = fem::int0;
  arr_2d<5, 3, double> pd(fem::fill0);
  double x0 = fem::double0;
  double y0 = fem::double0;
  double a = fem::double0;
  double b = fem::double0;
  double c = fem::double0;
  double d = fem::double0;
  double ad = fem::double0;
  double bc = fem::double0;
  double dlt = fem::double0;
  double ap = fem::double0;
  double bp = fem::double0;
  double cp = fem::double0;
  double dp = fem::double0;
  double aa = fem::double0;
  double act2 = fem::double0;
  double cc = fem::double0;
  double ab = fem::double0;
  double adbc = fem::double0;
  double cd = fem::double0;
  double bb = fem::double0;
  double bdt2 = fem::double0;
  double dd = fem::double0;
  arr_1d<3, double> zu(fem::fill0);
  arr_1d<3, double> zv(fem::fill0);
  arr_1d<3, double> zuu(fem::fill0);
  arr_1d<3, double> zuv(fem::fill0);
  arr_1d<3, double> zvv(fem::fill0);
  double p00 = fem::double0;
  double p10 = fem::double0;
  double p01 = fem::double0;
  double p20 = fem::double0;
  double p11 = fem::double0;
  double p02 = fem::double0;
  double h1 = fem::double0;
  double h2 = fem::double0;
  double h3 = fem::double0;
  double p30 = fem::double0;
  double p40 = fem::double0;
  double p50 = fem::double0;
  double p03 = fem::double0;
  double p04 = fem::double0;
  double p05 = fem::double0;
  double lusq = fem::double0;
  double lvsq = fem::double0;
  double spuv = fem::double0;
  double p41 = fem::double0;
  double p14 = fem::double0;
  double p21 = fem::double0;
  double p31 = fem::double0;
  double p12 = fem::double0;
  double p13 = fem::double0;
  double e1 = fem::double0;
  double e2 = fem::double0;
  double g1 = fem::double0;
  double g2 = fem::double0;
  double p22 = fem::double0;
  double p32 = fem::double0;
  double p23 = fem::double0;
  double dx = fem::double0;
  double dy = fem::double0;
  double u = fem::double0;
  double v = fem::double0;
  double p0 = fem::double0;
  double p1 = fem::double0;
  double p2 = fem::double0;
  double p3 = fem::double0;
  double p4 = fem::double0;
  double p5 = fem::double0;
  double z0 = fem::double0;
  int ir = fem::int0;
  int ili = fem::int0;
  double zii = fem::double0;
  double zii1 = fem::double0;
  double wt2 = fem::double0;
  double zii2 = fem::double0;
  double wt1 = fem::double0;
  FEM_DO_SAFE(iip, 1, nip) {
    xii = xi(iip);
    yii = yi(iip);
    ktlii = ktli(iip);
    itlii = itli(iip);
    if (iip == 1) {
      ktlipv = 0;
      itlipv = 0;
    }
    else {
      ktlipv = ktli(iip - 1);
      itlipv = itli(iip - 1);
    }
    //C Part 1.  Calculation of ZII by interpolation
    if (ktlii == 1) {
      //C Calculates the coefficients when necessary.
      if (ktlii != ktlipv || itlii != itlipv) {
        //C Loads coordinate and partial derivative values at the
        //C vertexes.
        FEM_DO_SAFE(i, 1, 3) {
          idp = ipt(i, itlii);
          x(i) = xd(idp);
          y(i) = yd(idp);
          z(i) = zd(idp);
          FEM_DO_SAFE(k, 1, 5) {
            pd(k, i) = pdd(k, idp);
          }
        }
        //C Determines the coefficients for the coordinate system
        //C transformation from the x-y system to the u-v system
        //C and vice versa.
        x0 = x(1);
        y0 = y(1);
        a = x(2) - x0;
        b = x(3) - x0;
        c = y(2) - y0;
        d = y(3) - y0;
        ad = a * d;
        bc = b * c;
        dlt = ad - bc;
        ap = d / dlt;
        bp = -b / dlt;
        cp = -c / dlt;
        dp = a / dlt;
        //C Converts the partial derivatives at the vertexes of the
        //C triangle for the u-v coordinate system.
        aa = a * a;
        act2 = 2.0f * a * c;
        cc = c * c;
        ab = a * b;
        adbc = ad + bc;
        cd = c * d;
        bb = b * b;
        bdt2 = 2.0f * b * d;
        dd = d * d;
        FEM_DO_SAFE(i, 1, 3) {
          zu(i) = a * pd(1, i) + c * pd(2, i);
          zv(i) = b * pd(1, i) + d * pd(2, i);
          zuu(i) = aa * pd(3, i) + act2 * pd(4, i) + cc * pd(5, i);
          zuv(i) = ab * pd(3, i) + adbc * pd(4, i) + cd * pd(5, i);
          zvv(i) = bb * pd(3, i) + bdt2 * pd(4, i) + dd * pd(5, i);
        }
        //C Calculates the coefficients of the polynomial.
        p00 = z(1);
        p10 = zu(1);
        p01 = zv(1);
        p20 = 0.5f * zuu(1);
        p11 = zuv(1);
        p02 = 0.5f * zvv(1);
        h1 = z(2) - p00 - p10 - p20;
        h2 = zu(2) - p10 - zuu(1);
        h3 = zuu(2) - zuu(1);
        p30 = 10.0f * h1 - 4.0f * h2 + 0.5f * h3;
        p40 = -15.0f * h1 + 7.0f * h2 - h3;
        p50 = 6.0f * h1 - 3.0f * h2 + 0.5f * h3;
        h1 = z(3) - p00 - p01 - p02;
        h2 = zv(3) - p01 - zvv(1);
        h3 = zvv(3) - zvv(1);
        p03 = 10.0f * h1 - 4.0f * h2 + 0.5f * h3;
        p04 = -15.0f * h1 + 7.0f * h2 - h3;
        p05 = 6.0f * h1 - 3.0f * h2 + 0.5f * h3;
        lusq = aa + cc;
        lvsq = bb + dd;
        spuv = ab + cd;
        p41 = 5.0f * spuv / lusq * p50;
        p14 = 5.0f * spuv / lvsq * p05;
        h1 = zv(2) - p01 - p11 - p41;
        h2 = zuv(2) - p11 - 4.0f * p41;
        p21 = 3.0f * h1 - h2;
        p31 = -2.0f * h1 + h2;
        h1 = zu(3) - p10 - p11 - p14;
        h2 = zuv(3) - p11 - 4.0f * p14;
        p12 = 3.0f * h1 - h2;
        p13 = -2.0f * h1 + h2;
        e1 = (lvsq - spuv) / ((lvsq - spuv) + (lusq - spuv));
        e2 = 1.0f - e1;
        g1 = 5.0f * e1 - 2.0f;
        g2 = 1.0f - g1;
        h1 = 5.0f * (e1 * (p50 - p41) + e2 * (p05 - p14)) + (p41 + p14);
        h2 = 0.5f * zvv(2) - p02 - p12;
        h3 = 0.5f * zuu(3) - p20 - p21;
        p22 = h1 + g1 * h2 + g2 * h3;
        p32 = h2 - p22;
        p23 = h3 - p22;
      }
      //C Converts XII and YII to u-v system.
      dx = xii - x0;
      dy = yii - y0;
      u = ap * dx + bp * dy;
      v = cp * dx + dp * dy;
      //C Evaluates the polynomial.
      p0 = p00 + v * (p01 + v * (p02 + v * (p03 + v * (p04 + v * p05))));
      p1 = p10 + v * (p11 + v * (p12 + v * (p13 + v * p14)));
      p2 = p20 + v * (p21 + v * (p22 + v * p23));
      p3 = p30 + v * (p31 + v * p32);
      p4 = p40 + v * p41;
      p5 = p50;
      zi(iip) = p0 + u * (p1 + u * (p2 + u * (p3 + u * (p4 + u * p5))));
      extrpi(iip) = false;
    }
    //C Part 2.  Calculation of ZII by extrapolation in the rectangle
    if (ktlii == 2) {
      //C Calculates the coefficients when necessary.
      if (ktlii != ktlipv || itlii != itlipv) {
        //C Loads coordinate and partial derivative values at the end
        //C points of the border line segment.
        FEM_DO_SAFE(i, 1, 2) {
          idp = ipl(i, itlii);
          x(i) = xd(idp);
          y(i) = yd(idp);
          z(i) = zd(idp);
          FEM_DO_SAFE(k, 1, 5) {
            pd(k, i) = pdd(k, idp);
          }
        }
        //C Determines the coefficients for the coordinate system
        //C transformation from the x-y system to the u-v system
        //C and vice versa.
        x0 = x(1);
        y0 = y(1);
        a = y(2) - y(1);
        b = x(2) - x(1);
        c = -b;
        d = a;
        ad = a * d;
        bc = b * c;
        dlt = ad - bc;
        ap = d / dlt;
        bp = -b / dlt;
        cp = -bp;
        dp = ap;
        //C Converts the partial derivatives at the end points of the
        //C border line segment for the u-v coordinate system.
        aa = a * a;
        act2 = 2.0f * a * c;
        cc = c * c;
        ab = a * b;
        adbc = ad + bc;
        cd = c * d;
        bb = b * b;
        bdt2 = 2.0f * b * d;
        dd = d * d;
        FEM_DO_SAFE(i, 1, 2) {
          zu(i) = a * pd(1, i) + c * pd(2, i);
          zv(i) = b * pd(1, i) + d * pd(2, i);
          zuu(i) = aa * pd(3, i) + act2 * pd(4, i) + cc * pd(5, i);
          zuv(i) = ab * pd(3, i) + adbc * pd(4, i) + cd * pd(5, i);
          zvv(i) = bb * pd(3, i) + bdt2 * pd(4, i) + dd * pd(5, i);
        }
        //C Calculates the coefficients of the polynomial.
        p00 = z(1);
        p10 = zu(1);
        p01 = zv(1);
        p20 = 0.5f * zuu(1);
        p11 = zuv(1);
        p02 = 0.5f * zvv(1);
        h1 = z(2) - p00 - p01 - p02;
        h2 = zv(2) - p01 - zvv(1);
        h3 = zvv(2) - zvv(1);
        p03 = 10.0f * h1 - 4.0f * h2 + 0.5f * h3;
        p04 = -15.0f * h1 + 7.0f * h2 - h3;
        p05 = 6.0f * h1 - 3.0f * h2 + 0.5f * h3;
        h1 = zu(2) - p10 - p11;
        h2 = zuv(2) - p11;
        p12 = 3.0f * h1 - h2;
        p13 = -2.0f * h1 + h2;
        p21 = 0.5f * (zuu(2) - zuu(1));
      }
      //C Converts XII and YII to u-v system.
      dx = xii - x0;
      dy = yii - y0;
      u = ap * dx + bp * dy;
      v = cp * dx + dp * dy;
      //C Evaluates the polynomial.
      p0 = p00 + v * (p01 + v * (p02 + v * (p03 + v * (p04 + v * p05))));
      p1 = p10 + v * (p11 + v * (p12 + v * p13));
      p2 = p20 + v * p21;
      zi(iip) = p0 + u * (p1 + u * p2);
      extrpi(iip) = true;
    }
    //C Part 3.  Calculation of ZII by extrapolation in the triangle
    if (ktlii == 3) {
      //C Calculates the coefficients when necessary.
      if (ktlii != ktlipv || itlii != itlipv) {
        //C Loads coordinate and partial derivative values at the vertex
        //C of the triangle.
        idp = ipl(1, itlii);
        x0 = xd(idp);
        y0 = yd(idp);
        z0 = zd(idp);
        FEM_DO_SAFE(k, 1, 5) {
          pd(k, 1) = pdd(k, idp);
        }
        //C Calculates the coefficients of the polynomial.
        p00 = z0;
        p10 = pd(1, 1);
        p01 = pd(2, 1);
        p20 = 0.5f * pd(3, 1);
        p11 = pd(4, 1);
        p02 = 0.5f * pd(5, 1);
      }
      //C Converts XII and YII to U-V system.
      u = xii - x0;
      v = yii - y0;
      //C Evaluates the polynomial.
      p0 = p00 + v * (p01 + v * p02);
      p1 = p10 + v * p11;
      zi(iip) = p0 + u * (p1 + u * p20);
      extrpi(iip) = true;
    }
    //C Part 4.  Calculation of ZII by extrapolation in the triangle
    //C          which is an overlap of two rectangles.
    if (ktlii == 4) {
      //C Calculates the coefficients.
      FEM_DO_SAFE(ir, 1, 2) {
        if (ir == 1) {
          ili = fem::mod(itlii + nl - 2, nl) + 1;
        }
        else {
          ili = itlii;
        }
        //C Loads coordinate and partial derivative values at the end
        //C points of the border line segment.
        FEM_DO_SAFE(i, 1, 2) {
          idp = ipl(i, ili);
          x(i) = xd(idp);
          y(i) = yd(idp);
          z(i) = zd(idp);
          FEM_DO_SAFE(k, 1, 5) {
            pd(k, i) = pdd(k, idp);
          }
        }
        //C Determines the coefficients for the coordinate system
        //C transformation from the x-y system to the u-v system
        //C and vice versa.
        x0 = x(1);
        y0 = y(1);
        a = y(2) - y(1);
        b = x(2) - x(1);
        c = -b;
        d = a;
        ad = a * d;
        bc = b * c;
        dlt = ad - bc;
        ap = d / dlt;
        bp = -b / dlt;
        cp = -bp;
        dp = ap;
        //C Converts the partial derivatives at the end points of the
        //C border line segment for the u-v coordinate system.
        aa = a * a;
        act2 = 2.0f * a * c;
        cc = c * c;
        ab = a * b;
        adbc = ad + bc;
        cd = c * d;
        bb = b * b;
        bdt2 = 2.0f * b * d;
        dd = d * d;
        FEM_DO_SAFE(i, 1, 2) {
          zu(i) = a * pd(1, i) + c * pd(2, i);
          zv(i) = b * pd(1, i) + d * pd(2, i);
          zuu(i) = aa * pd(3, i) + act2 * pd(4, i) + cc * pd(5, i);
          zuv(i) = ab * pd(3, i) + adbc * pd(4, i) + cd * pd(5, i);
          zvv(i) = bb * pd(3, i) + bdt2 * pd(4, i) + dd * pd(5, i);
        }
        //C Calculates the coefficients of the polynomial.
        p00 = z(1);
        p10 = zu(1);
        p01 = zv(1);
        p20 = 0.5f * zuu(1);
        p11 = zuv(1);
        p02 = 0.5f * zvv(1);
        h1 = z(2) - p00 - p01 - p02;
        h2 = zv(2) - p01 - zvv(1);
        h3 = zvv(2) - zvv(1);
        p03 = 10.0f * h1 - 4.0f * h2 + 0.5f * h3;
        p04 = -15.0f * h1 + 7.0f * h2 - h3;
        p05 = 6.0f * h1 - 3.0f * h2 + 0.5f * h3;
        h1 = zu(2) - p10 - p11;
        h2 = zuv(2) - p11;
        p12 = 3.0f * h1 - h2;
        p13 = -2.0f * h1 + h2;
        p21 = 0.5f * (zuu(2) - zuu(1));
        //C Converts XII and YII to u-v system.
        dx = xii - x0;
        dy = yii - y0;
        u = ap * dx + bp * dy;
        v = cp * dx + dp * dy;
        //C Evaluates the polynomial.
        p0 = p00 + v * (p01 + v * (p02 + v * (p03 + v * (p04 + v * p05))));
        p1 = p10 + v * (p11 + v * (p12 + v * p13));
        p2 = p20 + v * p21;
        zii = p0 + u * (p1 + u * p2);
        if (ir == 1) {
          zii1 = zii;
          wt2 = fem::pow2(((x(1) - x(2)) * (xii - x(2)) + (y(1) - y(
            2)) * (yii - y(2))));
        }
        else {
          zii2 = zii;
          wt1 = fem::pow2(((x(2) - x(1)) * (xii - x(1)) + (y(2) - y(
            1)) * (yii - y(1))));
        }
      }
      zi(iip) = (wt1 * zii1 + wt2 * zii2) / (wt1 + wt2);
      extrpi(iip) = true;
    }
  }
}

struct sdbi3p_save
{
  int ndppv;
  int nl;
  int nt;

  sdbi3p_save() :
    ndppv(fem::int0),
    nl(fem::int0),
    nt(fem::int0)
  {}
};

void
sdbi3p(
  common& cmn,
  int const& md,
  int const& ndp,
  arr_ref<double> xd,
  arr_ref<double> yd,
  arr_cref<double> zd,
  int const& nip,
  arr_cref<double> xi,
  arr_cref<double> yi,
  arr_ref<double> zi,
  int& ier,
  arr_ref<double, 2> wk,
  arr_ref<int, 2> iwk,
  arr_ref<bool> extrpi,
  arr_ref<int> near,
  arr_ref<int> next,
  arr_ref<double> dist)
{
  FEM_CMN_SVE(sdbi3p);
  xd(dimension(ndp));
  yd(dimension(ndp));
  zd(dimension(ndp));
  xi(dimension(nip));
  yi(dimension(nip));
  zi(dimension(nip));
  wk(dimension(ndp, 17));
  iwk(dimension(ndp, 25));
  extrpi(dimension(nip));
  near(dimension(ndp));
  next(dimension(ndp));
  dist(dimension(ndp));
  int& ndppv = sve.ndppv;
  int& nl = sve.nl;
  int& nt = sve.nt;
  int iert = fem::int0;
  int iip = fem::int0;
  const int nipimx = 51;
  int nipi = fem::int0;
  arr_1d<nipimx, int> ktli(fem::fill0);
  arr_1d<nipimx, int> itli(fem::fill0);
  //C
  //C Scattered-data bivariate interpolation
  //C (a master subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine performs bivariate interpolation when the data
  //C points are scattered in the x-y plane.  It is based on the
  //C revised Akima method that has the accuracy of a cubic (third-
  //C degree) polynomial.
  //C
  //C The input arguments are
  //C   MD  = mode of computation
  //C       = 1 for new XD-YD (default)
  //C       = 2 for old XD-YD, new ZD
  //C       = 3 for old XD-YD, old ZD,
  //C   NDP = number of data points (must be 10 or greater),
  //C   XD  = array of dimension NDP containing the x coordinates
  //C         of the data points,
  //C   YD  = array of dimension NDP containing the y coordinates
  //C         of the data points,
  //C   ZD  = array of dimension NDP containing the z values at
  //C         the data points,
  //C   NIP = number of output points at which interpolation is
  //C         to be performed (must be 1 or greater),
  //C   XI  = array of dimension NIP containing the x coordinates
  //C         of the output points,
  //C   YI  = array of dimension NIP containing the y coordinates
  //C         of the output points.
  //C
  //C The output arguments are
  //C   ZI  = array of dimension NIP, where interpolated z values
  //C         are to be stored,
  //C   IER = error flag
  //C       = 0 for no errors
  //C       = 1 for NDP = 9 or less
  //C       = 2 for NDP not equal to NDPPV
  //C       = 3 for NIP = 0 or less
  //C       = 9 for errors in SDTRAN called by this subroutine.
  //C
  //C The other arguments are
  //C   WK  = two-dimensional array of dimension NDP*17 used
  //C         internally as a work area,
  //C   IWK = two-dimensional integer array of dimension NDP*25
  //C         used internally as a work area.
  //C
  //C agebhard@uni-klu.ac.at: added from new TRIPACK:
  //C   NEAR, NEXT, DIST work arrays from TRMESH, size NDP
  //C
  //C The very first call to this subroutine and the call with a new
  //C NDP value or new XD and YD arrays must be made with MD=1.  The
  //C call with MD=2 must be preceded by another call with the same
  //C NDP value and same XD and YD arrays.  The call with MD=3 must
  //C be preceded by another call with the same NDP value and same
  //C XD, YD, and ZD arrays.  Between the call with MD=2 and its
  //C preceding call, the IWK array must not be disturbed.  Between
  //C the call with MD=3 and its preceding call, the WK and IWK
  //C arrays must not be disturbed.
  //C
  //C The user of this subroutine can save the storage, by NDP*6
  //C numerical storage units, by placing the statement
  //C     EQUIVALENCE (WK(1,1),IWK(1,20))
  //C in the program that calls this subroutine.
  //C
  //C The constant in the PARAMETER statement below is
  //C   NIPIMX = maximum number of output points to be processed
  //C            at a time.
  //C The constant value has been selected empirically.
  //C
  //C This subroutine calls the SDTRAN, SDPD3P, SDLCTN, and SDPLNL
  //C subroutines.
  //C
  //C Specification statements
  //C     .. Parameters ..
  //C     ..
  //C     .. Scalar Arguments ..
  //C
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Local Arrays ..
  //C     ..
  //C     .. External Subroutines ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C     .. Save statement ..
  //C     ..
  //C Error check
  if (ndp <= 9) {
    goto statement_20;
  }
  if (md != 2 && md != 3) {
    ndppv = ndp;
  }
  else {
    if (ndp != ndppv) {
      goto statement_30;
    }
  }
  if (nip <= 0) {
    goto statement_40;
  }
  //C Triangulates the x-y plane.  (for MD=1)
  if (md != 2 && md != 3) {
    sdtran(cmn, ndp, xd, yd, nt, iwk(1, 1), nl, iwk(1, 7), iert, iwk(1,
      1), iwk(1, 7), iwk(1, 13), iwk(1, 14), iwk(1, 9), near, next,
      dist);
    //C         CALL SDTRAN(NDP,XD,YD, NT,IPT,NL,IPL,IERT,
    //C    +                LIST,LPTR,LEND,LTRI,ITL)
    if (iert > 0) {
      goto statement_50;
    }
  }
  //C Estimates partial derivatives at all data points.  (for MD=1,2)
  if (md != 3) {
    sdpd3p(ndp, xd, yd, zd, wk(1, 1), wk(1, 6), wk(1, 15), wk(1, 17),
      iwk(1, 9), iwk(1, 10), iwk(1, 19));
    //C         CALL SDPD3P(NDP,XD,YD,ZD, PDD, CF3,CFL1,DSQ,IDSQ,IPC,NCP)
  }
  //C Locates all points at which interpolation is to be performed
  //C and interpolates the ZI values.  (for MD=1,2,3)
  FEM_DOSTEP(iip, 1, nip, nipimx) {
    nipi = fem::min(nip - iip + 1, nipimx);
    sdlctn(ndp, xd, yd, nt, iwk(1, 1), nl, iwk(1, 7), nipi, xi(iip),
      yi(iip), ktli, itli);
    //C         CALL SDLCTN(NDP,XD,YD,NT,IPT,NL,IPL,NIP,XI,YI, KTLI,ITLI)
    sdplnl(ndp, xd, yd, zd, nt, iwk(1, 1), nl, iwk(1, 7), wk(1, 1),
      nipi, xi(iip), yi(iip), ktli, itli, zi(iip), extrpi(iip));
    //C         CALL SDPLNL(NDP,XD,YD,ZD,NT,IPT,NL,IPL,PDD,
    //C    +                NIP,XI,YI,KTLI,ITLI, ZI)
  }
  //C Normal return
  ier = 0;
  return;
  //C Error exit
  statement_20:
  //C     WRITE (*,FMT=9000) MD,NDP
  ier = 1;
  return;
  statement_30:
  //C     WRITE (*,FMT=9010) MD,NDP,NDPPV
  ier = 2;
  return;
  statement_40:
  //C     WRITE (*,FMT=9020) MD,NDP,NIP
  ier = 3;
  return;
  statement_50:
  //C     WRITE (*,FMT=9030)
  ier = 9;
  //C Format statement for error message
}

struct sdsf3p_save
{
  int ndppv;
  int nl;
  int nt;

  sdsf3p_save() :
    ndppv(fem::int0),
    nl(fem::int0),
    nt(fem::int0)
  {}
};

void
sdsf3p(
  common& cmn,
  int const& md,
  int const& ndp,
  arr_ref<double> xd,
  arr_ref<double> yd,
  arr_cref<double> zd,
  int const& nxi,
  arr_cref<double> xi,
  int const& nyi,
  arr_cref<double> yi,
  arr_ref<double, 2> zi,
  int& ier,
  arr_ref<double, 2> wk,
  arr_ref<int, 2> iwk,
  arr_ref<bool, 2> extrpi,
  arr_ref<int> near,
  arr_ref<int> next,
  arr_ref<double> dist)
{
  FEM_CMN_SVE(sdsf3p);
  xd(dimension(ndp));
  yd(dimension(ndp));
  zd(dimension(ndp));
  xi(dimension(nxi));
  yi(dimension(nyi));
  zi(dimension(nxi, nyi));
  wk(dimension(ndp, 17));
  iwk(dimension(ndp, 25));
  extrpi(dimension(nxi, nyi));
  near(dimension(ndp));
  next(dimension(ndp));
  dist(dimension(ndp));
  int& ndppv = sve.ndppv;
  int& nl = sve.nl;
  int& nt = sve.nt;
  int iert = fem::int0;
  int iyi = fem::int0;
  int iip = fem::int0;
  const int nipimx = 51;
  arr_1d<nipimx, double> yii(fem::fill0);
  int ixi = fem::int0;
  int nipi = fem::int0;
  arr_1d<nipimx, int> ktli(fem::fill0);
  arr_1d<nipimx, int> itli(fem::fill0);
  //C
  //C Scattered-data smooth surface fitting
  //C (a master subroutine of the SDBI3P/SDSF3P subroutine package)
  //C
  //C Hiroshi Akima
  //C U.S. Department of Commerce, NTIA/ITS
  //C Version of 1995/05
  //C
  //C This subroutine performs smooth surface fitting when the data
  //C points are scattered in the x-y plane.  It is based on the
  //C revised Akima method that has the accuracy of a cubic (third-
  //C degree) polynomial.
  //C
  //C The input arguments are
  //C   MD  = mode of computation
  //C       = 1 for new XD-YD (default)
  //C       = 2 for old XD-YD, new ZD
  //C       = 3 for old XD-YD, old ZD,
  //C   NDP = number of data points (must be 10 or greater),
  //C   XD  = array of dimension NDP containing the x coordinates
  //C         of the data points,
  //C   YD  = array of dimension NDP containing the y coordinates
  //C         of the data points,
  //C   ZD  = array of dimension NDP containing the z values at
  //C         the data points,
  //C   NXI = number of output grid points in the x coordinate
  //C         (must be 1 or greater),
  //C   XI  = array of dimension NXI containing the x coordinates
  //C         of the output grid points,
  //C   NYI = number of output grid points in the y coordinate
  //C         (must be 1 or greater),
  //C   YI  = array of dimension NYI containing the y coordinates
  //C         of the output grid points.
  //C
  //C The output arguments are
  //C   ZI  = two-dimensional array of dimension NXI*NYI, where
  //C         the interpolated z values at the output grid points
  //C         are to be stored,
  //C   IER = error flag
  //C       = 0 for no errors
  //C       = 1 for NDP = 9 or less
  //C       = 2 for NDP not equal to NDPPV
  //C       = 3 for NXI = 0 or less
  //C       = 4 for NYI = 0 or less
  //C       = 9 for errors in SDTRAN called by this subroutine.
  //C
  //C The other arguments are
  //C   WK  = two-dimensional array of dimension NDP*36 used
  //C         internally as a work area,
  //C   IWK = two-dimensional integer array of dimension NDP*25
  //C         used internally as a work area.
  //C
  //C agebhard@uni-klu.ac.at: added from new TRIPACK:
  //C   NEAR, NEXT, DIST work arrays from TRMESH, size NDP
  //C
  //C The very first call to this subroutine and the call with a new
  //C NDP value or new XD and YD arrays must be made with MD=1.  The
  //C call with MD=2 must be preceded by another call with the same
  //C NDP value and same XD and YD arrays.  The call with MD=3 must
  //C be preceded by another call with the same NDP value and same
  //C XD, YD, and ZD arrays.  Between the call with MD=2 and its
  //C preceding call, the IWK array must not be disturbed.  Between
  //C the call with MD=3 and its preceding call, the WK and IWK
  //C arrays must not be disturbed.
  //C
  //C The user of this subroutine can save the storage, by NDP*6
  //C numeric storage units, by placing the statement
  //C     EQUIVALENCE (WK(1,1),IWK(1,20))
  //C in the program that calls this subroutine.
  //C
  //C The constant in the PARAMETER statement below is
  //C   NIPIMX = maximum number of output points to be processed
  //C            at a time.
  //C The constant value has been selected empirically.
  //C
  //C This subroutine calls the SDTRAN, SDPD3P, SDLCTN, and SDPLNL
  //C subroutines.
  //C
  //C Specification statements
  //C     .. Parameters ..
  //C     ..
  //C     .. Scalar Arguments ..
  //C     ..
  //C     .. Array Arguments ..
  //C     ..
  //C     .. Local Scalars ..
  //C     ..
  //C     .. Local Arrays ..
  //C     ..
  //C     .. External Subroutines ..
  //C     ..
  //C     .. Intrinsic Functions ..
  //C     ..
  //C     .. Save statement ..
  //C     ..
  //C Error check
  if (ndp <= 9) {
    goto statement_40;
  }
  if (md != 2 && md != 3) {
    ndppv = ndp;
  }
  else {
    if (ndp != ndppv) {
      goto statement_50;
    }
  }
  if (nxi <= 0) {
    goto statement_60;
  }
  if (nyi <= 0) {
    goto statement_70;
  }
  //C Triangulates the x-y plane.  (for MD=1)
  if (md != 2 && md != 3) {
    sdtran(cmn, ndp, xd, yd, nt, iwk(1, 1), nl, iwk(1, 7), iert, iwk(1,
      1), iwk(1, 7), iwk(1, 13), iwk(1, 14), iwk(1, 9), near, next,
      dist);
    //C         CALL SDTRAN(NDP,XD,YD, NT,IPT,NL,IPL,IERT,
    //C    +                LIST,LPTR,LEND,LTRI,ITL)
    if (iert > 0) {
      goto statement_80;
    }
  }
  //C Estimates partial derivatives at all data points.  (for MD=1,2)
  if (md != 3) {
    sdpd3p(ndp, xd, yd, zd, wk(1, 1), wk(1, 6), wk(1, 15), wk(1, 17),
      iwk(1, 9), iwk(1, 10), iwk(1, 19));
    //C         CALL SDPD3P(NDP,XD,YD,ZD, PDD, CF3,CFL1,DSQ,IDSQ,IPC,NCP)
  }
  //C Locates all grid points at which interpolation is to be
  //C performed and interpolates the ZI values.  (for MD=1,2,3)
  FEM_DO_SAFE(iyi, 1, nyi) {
    FEM_DO_SAFE(iip, 1, nipimx) {
      yii(iip) = yi(iyi);
    }
    FEM_DOSTEP(ixi, 1, nxi, nipimx) {
      nipi = fem::min(nxi - ixi + 1, nipimx);
      sdlctn(ndp, xd, yd, nt, iwk(1, 1), nl, iwk(1, 7), nipi, xi(ixi),
        yii, ktli, itli);
      //C             CALL SDLCTN(NDP,XD,YD,NT,IPT,NL,IPL,NIP,XI,YI, KTLI,ITLI)
      sdplnl(ndp, xd, yd, zd, nt, iwk(1, 1), nl, iwk(1, 7), wk(1, 1),
        nipi, xi(ixi), yii, ktli, itli, zi(ixi, iyi), extrpi(ixi,
        iyi));
      //C             CALL SDPLNL(NDP,XD,YD,ZD,NT,ITP,NL,IPL,PDD,
      //C    +                    NIP,XI,YI,KTLI,ITLI, ZI)
    }
  }
  //C Normal return
  ier = 0;
  return;
  //C Error exit
  statement_40:
  //C     WRITE (*,FMT=9000) MD,NDP
  ier = 1;
  return;
  statement_50:
  //C     WRITE (*,FMT=9010) MD,NDP,NDPPV
  ier = 2;
  return;
  statement_60:
  //C     WRITE (*,FMT=9020) MD,NDP,NXI,NYI
  ier = 3;
  return;
  statement_70:
  //C     WRITE (*,FMT=9030) MD,NDP,NXI,NYI
  ier = 4;
  return;
  statement_80:
  //C     WRITE (*,FMT=9040)
  ier = 9;
  //C Format statement for error message
}

void
optim(
  common& cmn,
  arr_cref<double> x,
  arr_cref<double> y,
  int const& na,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& nit,
  arr_ref<int, 2> iwk,
  int& ier)
{
  x(dimension(star));
  y(dimension(star));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(star));
  iwk(dimension(2, na));
  int nna = fem::int0;
  int maxit = fem::int0;
  int iter = fem::int0;
  bool swp = fem::bool0;
  int i = fem::int0;
  int io1 = fem::int0;
  int io2 = fem::int0;
  int lpl = fem::int0;
  int lpp = fem::int0;
  int lp = fem::int0;
  int n2 = fem::int0;
  int n1 = fem::int0;
  int lp21 = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   06/27/98
  //C
  //C   Given a set of NA triangulation arcs, this subroutine
  //C optimizes the portion of the triangulation consisting of
  //C the quadrilaterals (pairs of adjacent triangles) which
  //C have the arcs as diagonals by applying the circumcircle
  //C test and appropriate swaps to the arcs.
  //C
  //C   An iteration consists of applying the swap test and
  //C swaps to all NA arcs in the order in which they are
  //C stored.  The iteration is repeated until no swap occurs
  //C or NIT iterations have been performed.  The bound on the
  //C number of iterations may be necessary to prevent an
  //C infinite loop caused by cycling (reversing the effect of a
  //C previous swap) due to floating point inaccuracy when four
  //C or more nodes are nearly cocircular.
  //C
  //C On input:
  //C
  //C       X,Y = Arrays containing the nodal coordinates.
  //C
  //C       NA = Number of arcs in the set.  NA .GE. 0.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LIST,LPTR,LEND = Data structure defining the trian-
  //C                        gulation.  Refer to Subroutine
  //C                        TRMESH.
  //C
  //C       NIT = Maximum number of iterations to be performed.
  //C             A reasonable value is 3*NA.  NIT .GE. 1.
  //C
  //C       IWK = Integer array dimensioned 2 by NA containing
  //C             the nodal indexes of the arc endpoints (pairs
  //C             of endpoints are stored in columns).
  //C
  //C On output:
  //C
  //C       LIST,LPTR,LEND = Updated triangulation data struc-
  //C                        ture reflecting the swaps.
  //C
  //C       NIT = Number of iterations performed.
  //C
  //C       IWK = Endpoint indexes of the new set of arcs
  //C             reflecting the swaps.
  //C
  //C       IER = Error indicator:
  //C             IER = 0 if no errors were encountered.
  //C             IER = 1 if a swap occurred on the last of
  //C                     MAXIT iterations, where MAXIT is the
  //C                     value of NIT on input.  The new set
  //C                     of arcs in not necessarily optimal
  //C                     in this case.
  //C             IER = 2 if NA < 0 or NIT < 1 on input.
  //C             IER = 3 if IWK(2,I) is not a neighbor of
  //C                     IWK(1,I) for some I in the range 1
  //C                     to NA.  A swap may have occurred in
  //C                     this case.
  //C             IER = 4 if a zero pointer was returned by
  //C                     Subroutine SWAP.
  //C
  //C Modules required by OPTIM:  LSTPTR, SWAP, SWPTST
  //C
  //C Intrinsic function called by OPTIM:  ABS
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C I =       Column index for IWK
  //C IO1,IO2 = Nodal indexes of the endpoints of an arc in IWK
  //C ITER =    Iteration count
  //C LP =      LIST pointer
  //C LP21 =    Parameter returned by SWAP (not used)
  //C LPL =     Pointer to the last neighbor of IO1
  //C LPP =     Pointer to the node preceding IO2 as a neighbor
  //C             of IO1
  //C MAXIT =   Input value of NIT
  //C N1,N2 =   Nodes opposite IO1->IO2 and IO2->IO1,
  //C             respectively
  //C NNA =     Local copy of NA
  //C SWP =     Flag set to TRUE iff a swap occurs in the
  //C             optimization loop
  //C
  nna = na;
  maxit = nit;
  if (nna < 0 || maxit < 1) {
    goto statement_7;
  }
  //C
  //C Initialize iteration count ITER and test for NA = 0.
  //C
  iter = 0;
  if (nna == 0) {
    goto statement_5;
  }
  //C
  //C Top of loop --
  //C   SWP = TRUE iff a swap occurred in the current iteration.
  //C
  statement_1:
  if (iter == maxit) {
    goto statement_6;
  }
  iter++;
  swp = false;
  //C
  //C   Inner loop on arcs IO1-IO2 --
  //C
  FEM_DO_SAFE(i, 1, nna) {
    io1 = iwk(1, i);
    io2 = iwk(2, i);
    //C
    //C   Set N1 and N2 to the nodes opposite IO1->IO2 and
    //C     IO2->IO1, respectively.  Determine the following:
    //C
    //C     LPL = pointer to the last neighbor of IO1,
    //C     LP = pointer to IO2 as a neighbor of IO1, and
    //C     LPP = pointer to the node N2 preceding IO2.
    //C
    lpl = lend(io1);
    lpp = lpl;
    lp = lptr(lpp);
    statement_2:
    if (list(lp) == io2) {
      goto statement_3;
    }
    lpp = lp;
    lp = lptr(lpp);
    if (lp != lpl) {
      goto statement_2;
    }
    //C
    //C   IO2 should be the last neighbor of IO1.  Test for no
    //C     arc and bypass the swap test if IO1 is a boundary
    //C     node.
    //C
    if (fem::abs(list(lp)) != io2) {
      goto statement_8;
    }
    if (list(lp) < 0) {
      goto statement_4;
    }
    //C
    //C   Store N1 and N2, or bypass the swap test if IO1 is a
    //C     boundary node and IO2 is its first neighbor.
    //C
    statement_3:
    n2 = list(lpp);
    if (n2 < 0) {
      goto statement_4;
    }
    lp = lptr(lp);
    n1 = fem::abs(list(lp));
    //C
    //C   Test IO1-IO2 for a swap, and update IWK if necessary.
    //C
    if (!swptst(cmn, n1, n2, io1, io2, x, y)) {
      goto statement_4;
    }
    swap(n1, n2, io1, io2, list, lptr, lend, lp21);
    if (lp21 == 0) {
      goto statement_9;
    }
    swp = true;
    iwk(1, i) = n1;
    iwk(2, i) = n2;
    statement_4:;
  }
  if (swp) {
    goto statement_1;
  }
  //C
  //C Successful termination.
  //C
  statement_5:
  nit = iter;
  ier = 0;
  return;
  //C
  //C MAXIT iterations performed without convergence.
  //C
  statement_6:
  nit = maxit;
  ier = 1;
  return;
  //C
  //C Invalid input parameter.
  //C
  statement_7:
  nit = 0;
  ier = 2;
  return;
  //C
  //C IO2 is not a neighbor of IO1.
  //C
  statement_8:
  nit = iter;
  ier = 3;
  return;
  //C
  //C Zero pointer returned by SWAP.
  //C
  statement_9:
  nit = iter;
  ier = 4;
}

void
edge(
  common& cmn,
  int const& in1,
  int const& in2,
  arr_cref<double> x,
  arr_cref<double> y,
  int& lwk,
  arr_ref<int, 2> iwk,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& ier)
{
  x(dimension(star));
  y(dimension(star));
  iwk(dimension(2, star));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(star));
  int n1 = fem::int0;
  int n2 = fem::int0;
  int iwend = fem::int0;
  int lpl = fem::int0;
  int n0 = fem::int0;
  int lp = fem::int0;
  int iwl = fem::int0;
  int nit = fem::int0;
  double x1 = fem::double0;
  double y1 = fem::double0;
  double x2 = fem::double0;
  double y2 = fem::double0;
  int n1lst = fem::int0;
  int n1frst = fem::int0;
  int nl = fem::int0;
  int nr = fem::int0;
  double dx = fem::double0;
  double dy = fem::double0;
  int next = fem::int0;
  int iwf = fem::int0;
  int lft = fem::int0;
  double x0 = fem::double0;
  double y0 = fem::double0;
  int iwc = fem::int0;
  int iwcp1 = fem::int0;
  int lp21 = fem::int0;
  int i = fem::int0;
  int ierr = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   06/23/98
  //C
  //C   Given a triangulation of N nodes and a pair of nodal
  //C indexes IN1 and IN2, this routine swaps arcs as necessary
  //C to force IN1 and IN2 to be adjacent.  Only arcs which
  //C intersect IN1-IN2 are swapped out.  If a Delaunay triangu-
  //C lation is input, the resulting triangulation is as close
  //C as possible to a Delaunay triangulation in the sense that
  //C all arcs other than IN1-IN2 are locally optimal.
  //C
  //C   A sequence of calls to EDGE may be used to force the
  //C presence of a set of edges defining the boundary of a non-
  //C convex and/or multiply connected region (refer to Subrou-
  //C tine ADDCST), or to introduce barriers into the triangula-
  //C tion.  Note that Subroutine GETNP will not necessarily
  //C return closest nodes if the triangulation has been con-
  //C strained by a call to EDGE.  However, this is appropriate
  //C in some applications, such as triangle-based interpolation
  //C on a nonconvex domain.
  //C
  //C On input:
  //C
  //C       IN1,IN2 = Indexes (of X and Y) in the range 1 to N
  //C                 defining a pair of nodes to be connected
  //C                 by an arc.
  //C
  //C       X,Y = Arrays of length N containing the Cartesian
  //C             coordinates of the nodes.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LWK = Number of columns reserved for IWK.  This must
  //C             be at least NI -- the number of arcs which
  //C             intersect IN1-IN2.  (NI is bounded by N-3.)
  //C
  //C       IWK = Integer work array of length at least 2*LWK.
  //C
  //C       LIST,LPTR,LEND = Data structure defining the trian-
  //C                        gulation.  Refer to Subroutine
  //C                        TRMESH.
  //C
  //C On output:
  //C
  //C       LWK = Number of arcs which intersect IN1-IN2 (but
  //C             not more than the input value of LWK) unless
  //C             IER = 1 or IER = 3.  LWK = 0 if and only if
  //C             IN1 and IN2 were adjacent (or LWK=0) on input.
  //C
  //C       IWK = Array containing the indexes of the endpoints
  //C             of the new arcs other than IN1-IN2 unless IER
  //C             .GT. 0 or LWK = 0.  New arcs to the left of
  //C             IN2-IN1 are stored in the first K-1 columns
  //C             (left portion of IWK), column K contains
  //C             zeros, and new arcs to the right of IN2-IN1
  //C             occupy columns K+1,...,LWK.  (K can be deter-
  //C             mined by searching IWK for the zeros.)
  //C
  //C       LIST,LPTR,LEND = Data structure updated if necessary
  //C                        to reflect the presence of an arc
  //C                        connecting IN1 and IN2 unless IER
  //C                        .NE. 0.  The data structure has
  //C                        been altered if IER = 4.
  //C
  //C       IER = Error indicator:
  //C             IER = 0 if no errors were encountered.
  //C             IER = 1 if IN1 .LT. 1, IN2 .LT. 1, IN1 = IN2,
  //C                     or LWK .LT. 0 on input.
  //C             IER = 2 if more space is required in IWK.
  //C             IER = 3 if IN1 and IN2 could not be connected
  //C                     due to either an invalid data struc-
  //C                     ture or collinear nodes (and floating
  //C                     point error).
  //C             IER = 4 if an error flag was returned by
  //C                     OPTIM.
  //C
  //C   An error message is written to the standard output unit
  //C in the case of IER = 3 or IER = 4.
  //C
  //C Modules required by EDGE:  LEFT, LSTPTR, OPTIM, SWAP,
  //C                              SWPTST
  //C
  //C Intrinsic function called by EDGE:  ABS
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C DX,DY =   Components of arc N1-N2
  //C I =       DO-loop index and column index for IWK
  //C IERR =    Error flag returned by Subroutine OPTIM
  //C IWC =     IWK index between IWF and IWL -- NL->NR is
  //C             stored in IWK(1,IWC)->IWK(2,IWC)
  //C IWCP1 =   IWC + 1
  //C IWEND =   Input or output value of LWK
  //C IWF =     IWK (column) index of the first (leftmost) arc
  //C             which intersects IN1->IN2
  //C IWL =     IWK (column) index of the last (rightmost) are
  //C             which intersects IN1->IN2
  //C LFT =     Flag used to determine if a swap results in the
  //C             new arc intersecting IN1-IN2 -- LFT = 0 iff
  //C             N0 = IN1, LFT = -1 implies N0 LEFT IN1->IN2,
  //C             and LFT = 1 implies N0 LEFT IN2->IN1
  //C LP21 =    Unused parameter returned by SWAP
  //C LP =      List pointer (index) for LIST and LPTR
  //C LPL =     Pointer to the last neighbor of IN1 or NL
  //C N0 =      Neighbor of N1 or node opposite NR->NL
  //C N1,N2 =   Local copies of IN1 and IN2
  //C N1FRST =  First neighbor of IN1
  //C N1LST =   (Signed) last neighbor of IN1
  //C NEXT =    Node opposite NL->NR
  //C NIT =     Flag or number of iterations employed by OPTIM
  //C NL,NR =   Endpoints of an arc which intersects IN1-IN2
  //C             with NL LEFT IN1->IN2
  //C X0,Y0 =   Coordinates of N0
  //C X1,Y1 =   Coordinates of IN1
  //C X2,Y2 =   Coordinates of IN2
  //C
  //C Store IN1, IN2, and LWK in local variables and test for
  //C   errors.
  //C
  n1 = in1;
  n2 = in2;
  iwend = lwk;
  if (n1 < 1 || n2 < 1 || n1 == n2 || iwend < 0) {
    goto statement_31;
  }
  //C
  //C Test for N2 as a neighbor of N1.  LPL points to the last
  //C   neighbor of N1.
  //C
  lpl = lend(n1);
  n0 = fem::abs(list(lpl));
  lp = lpl;
  statement_1:
  if (n0 == n2) {
    goto statement_30;
  }
  lp = lptr(lp);
  n0 = list(lp);
  if (lp != lpl) {
    goto statement_1;
  }
  //C
  //C Initialize parameters.
  //C
  iwl = 0;
  nit = 0;
  //C
  //C Store the coordinates of N1 and N2.
  //C
  statement_2:
  x1 = x(n1);
  y1 = y(n1);
  x2 = x(n2);
  y2 = y(n2);
  //C
  //C Set NR and NL to adjacent neighbors of N1 such that
  //C   NR LEFT N2->N1 and NL LEFT N1->N2,
  //C   (NR Forward N1->N2 or NL Forward N1->N2), and
  //C   (NR Forward N2->N1 or NL Forward N2->N1).
  //C
  //C   Initialization:  Set N1FRST and N1LST to the first and
  //C     (signed) last neighbors of N1, respectively, and
  //C     initialize NL to N1FRST.
  //C
  lpl = lend(n1);
  n1lst = list(lpl);
  lp = lptr(lpl);
  n1frst = list(lp);
  nl = n1frst;
  if (n1lst < 0) {
    goto statement_4;
  }
  //C
  //C   N1 is an interior node.  Set NL to the first candidate
  //C     for NR (NL LEFT N2->N1).
  //C
  statement_3:
  if (left(x2, y2, x1, y1, x(nl), y(nl))) {
    goto statement_4;
  }
  lp = lptr(lp);
  nl = list(lp);
  if (nl != n1frst) {
    goto statement_3;
  }
  //C
  //C   All neighbors of N1 are strictly left of N1->N2.
  //C
  goto statement_5;
  //C
  //C   NL = LIST(LP) LEFT N2->N1.  Set NR to NL and NL to the
  //C     following neighbor of N1.
  //C
  statement_4:
  nr = nl;
  lp = lptr(lp);
  nl = fem::abs(list(lp));
  if (left(x1, y1, x2, y2, x(nl), y(nl))) {
    //C
    //C   NL LEFT N1->N2 and NR LEFT N2->N1.  The Forward tests
    //C     are employed to avoid an error associated with
    //C     collinear nodes.
    //C
    dx = x2 - x1;
    dy = y2 - y1;
    if ((dx * (x(nl) - x1) + dy * (y(nl) - y1) >= 0.f || dx * (x(
        nr) - x1) + dy * (y(nr) - y1) >= 0.f) && (dx * (x(nl) - x2) +
        dy * (y(nl) - y2) <= 0.f || dx * (x(nr) - x2) + dy * (y(nr) -
        y2) <= 0.f)) {
      goto statement_6;
    }
    //C
    //C   NL-NR does not intersect N1-N2.  However, there is
    //C     another candidate for the first arc if NL lies on
    //C     the line N1-N2.
    //C
    if (!left(x2, y2, x1, y1, x(nl), y(nl))) {
      goto statement_5;
    }
  }
  //C
  //C   Bottom of loop.
  //C
  if (nl != n1frst) {
    goto statement_4;
  }
  //C
  //C Either the triangulation is invalid or N1-N2 lies on the
  //C   convex hull boundary and an edge NR->NL (opposite N1 and
  //C   intersecting N1-N2) was not found due to floating point
  //C   error.  Try interchanging N1 and N2 -- NIT > 0 iff this
  //C   has already been done.
  //C
  statement_5:
  if (nit > 0) {
    goto statement_33;
  }
  nit = 1;
  n1 = n2;
  n2 = in1;
  goto statement_2;
  //C
  //C Store the ordered sequence of intersecting edges NL->NR in
  //C   IWK(1,IWL)->IWK(2,IWL).
  //C
  statement_6:
  iwl++;
  if (iwl > iwend) {
    goto statement_32;
  }
  iwk(1, iwl) = nl;
  iwk(2, iwl) = nr;
  //C
  //C   Set NEXT to the neighbor of NL which follows NR.
  //C
  lpl = lend(nl);
  lp = lptr(lpl);
  //C
  //C   Find NR as a neighbor of NL.  The search begins with
  //C     the first neighbor.
  //C
  statement_7:
  if (list(lp) == nr) {
    goto statement_8;
  }
  lp = lptr(lp);
  if (lp != lpl) {
    goto statement_7;
  }
  //C
  //C   NR must be the last neighbor, and NL->NR cannot be a
  //C     boundary edge.
  //C
  if (list(lp) != nr) {
    goto statement_33;
  }
  //C
  //C   Set NEXT to the neighbor following NR, and test for
  //C     termination of the store loop.
  //C
  statement_8:
  lp = lptr(lp);
  next = fem::abs(list(lp));
  if (next == n2) {
    goto statement_9;
  }
  //C
  //C   Set NL or NR to NEXT.
  //C
  if (left(x1, y1, x2, y2, x(next), y(next))) {
    nl = next;
  }
  else {
    nr = next;
  }
  goto statement_6;
  //C
  //C IWL is the number of arcs which intersect N1-N2.
  //C   Store LWK.
  //C
  statement_9:
  lwk = iwl;
  iwend = iwl;
  //C
  //C Initialize for edge swapping loop -- all possible swaps
  //C   are applied (even if the new arc again intersects
  //C   N1-N2), arcs to the left of N1->N2 are stored in the
  //C   left portion of IWK, and arcs to the right are stored in
  //C   the right portion.  IWF and IWL index the first and last
  //C   intersecting arcs.
  //C
  iwf = 1;
  //C
  //C Top of loop -- set N0 to N1 and NL->NR to the first edge.
  //C   IWC points to the arc currently being processed.  LFT
  //C   .LE. 0 iff N0 LEFT N1->N2.
  //C
  statement_10:
  lft = 0;
  n0 = n1;
  x0 = x1;
  y0 = y1;
  nl = iwk(1, iwf);
  nr = iwk(2, iwf);
  iwc = iwf;
  //C
  //C   Set NEXT to the node opposite NL->NR unless IWC is the
  //C     last arc.
  //C
  statement_11:
  if (iwc == iwl) {
    goto statement_21;
  }
  iwcp1 = iwc + 1;
  next = iwk(1, iwcp1);
  if (next != nl) {
    goto statement_16;
  }
  next = iwk(2, iwcp1);
  //C
  //C   NEXT RIGHT N1->N2 and IWC .LT. IWL.  Test for a possible
  //C     swap.
  //C
  if (!left(x0, y0, x(nr), y(nr), x(next), y(next))) {
    goto statement_14;
  }
  if (lft >= 0) {
    goto statement_12;
  }
  if (!left(x(nl), y(nl), x0, y0, x(next), y(next))) {
    goto statement_14;
  }
  //C
  //C   Replace NL->NR with N0->NEXT.
  //C
  swap(next, n0, nl, nr, list, lptr, lend, lp21);
  iwk(1, iwc) = n0;
  iwk(2, iwc) = next;
  goto statement_15;
  //C
  //C   Swap NL-NR for N0-NEXT, shift columns IWC+1,...,IWL to
  //C     the left, and store N0-NEXT in the right portion of
  //C     IWK.
  //C
  statement_12:
  swap(next, n0, nl, nr, list, lptr, lend, lp21);
  FEM_DO_SAFE(i, iwcp1, iwl) {
    iwk(1, i - 1) = iwk(1, i);
    iwk(2, i - 1) = iwk(2, i);
  }
  iwk(1, iwl) = n0;
  iwk(2, iwl) = next;
  iwl = iwl - 1;
  nr = next;
  goto statement_11;
  //C
  //C   A swap is not possible.  Set N0 to NR.
  //C
  statement_14:
  n0 = nr;
  x0 = x(n0);
  y0 = y(n0);
  lft = 1;
  //C
  //C   Advance to the next arc.
  //C
  statement_15:
  nr = next;
  iwc++;
  goto statement_11;
  //C
  //C   NEXT LEFT N1->N2, NEXT .NE. N2, and IWC .LT. IWL.
  //C     Test for a possible swap.
  //C
  statement_16:
  if (!left(x(nl), y(nl), x0, y0, x(next), y(next))) {
    goto statement_19;
  }
  if (lft <= 0) {
    goto statement_17;
  }
  if (!left(x0, y0, x(nr), y(nr), x(next), y(next))) {
    goto statement_19;
  }
  //C
  //C   Replace NL->NR with NEXT->N0.
  //C
  swap(next, n0, nl, nr, list, lptr, lend, lp21);
  iwk(1, iwc) = next;
  iwk(2, iwc) = n0;
  goto statement_20;
  //C
  //C   Swap NL-NR for N0-NEXT, shift columns IWF,...,IWC-1 to
  //C     the right, and store N0-NEXT in the left portion of
  //C     IWK.
  //C
  statement_17:
  swap(next, n0, nl, nr, list, lptr, lend, lp21);
  FEM_DOSTEP(i, iwc - 1, iwf, -1) {
    iwk(1, i + 1) = iwk(1, i);
    iwk(2, i + 1) = iwk(2, i);
  }
  iwk(1, iwf) = n0;
  iwk(2, iwf) = next;
  iwf++;
  goto statement_20;
  //C
  //C   A swap is not possible.  Set N0 to NL.
  //C
  statement_19:
  n0 = nl;
  x0 = x(n0);
  y0 = y(n0);
  lft = -1;
  //C
  //C   Advance to the next arc.
  //C
  statement_20:
  nl = next;
  iwc++;
  goto statement_11;
  //C
  //C   N2 is opposite NL->NR (IWC = IWL).
  //C
  statement_21:
  if (n0 == n1) {
    goto statement_24;
  }
  if (lft < 0) {
    goto statement_22;
  }
  //C
  //C   N0 RIGHT N1->N2.  Test for a possible swap.
  //C
  if (!left(x0, y0, x(nr), y(nr), x2, y2)) {
    goto statement_10;
  }
  //C
  //C   Swap NL-NR for N0-N2 and store N0-N2 in the right
  //C     portion of IWK.
  //C
  swap(n2, n0, nl, nr, list, lptr, lend, lp21);
  iwk(1, iwl) = n0;
  iwk(2, iwl) = n2;
  iwl = iwl - 1;
  goto statement_10;
  //C
  //C   N0 LEFT N1->N2.  Test for a possible swap.
  //C
  statement_22:
  if (!left(x(nl), y(nl), x0, y0, x2, y2)) {
    goto statement_10;
  }
  //C
  //C   Swap NL-NR for N0-N2, shift columns IWF,...,IWL-1 to the
  //C     right, and store N0-N2 in the left portion of IWK.
  //C
  swap(n2, n0, nl, nr, list, lptr, lend, lp21);
  i = iwl;
  statement_23:
  iwk(1, i) = iwk(1, i - 1);
  iwk(2, i) = iwk(2, i - 1);
  i = i - 1;
  if (i > iwf) {
    goto statement_23;
  }
  iwk(1, iwf) = n0;
  iwk(2, iwf) = n2;
  iwf++;
  goto statement_10;
  //C
  //C IWF = IWC = IWL.  Swap out the last arc for N1-N2 and
  //C   store zeros in IWK.
  //C
  statement_24:
  swap(n2, n1, nl, nr, list, lptr, lend, lp21);
  iwk(1, iwc) = 0;
  iwk(2, iwc) = 0;
  //C
  //C Optimization procedure --
  //C
  if (iwc > 1) {
    //C
    //C   Optimize the set of new arcs to the left of IN1->IN2.
    //C
    nit = 3 * (iwc - 1);
    optim(cmn, x, y, iwc - 1, list, lptr, lend, nit, iwk, ierr);
    if (ierr != 0) {
      goto statement_34;
    }
  }
  if (iwc < iwend) {
    //C
    //C   Optimize the set of new arcs to the right of IN1->IN2.
    //C
    nit = 3 * (iwend - iwc);
    optim(cmn, x, y, iwend - iwc, list, lptr, lend, nit, iwk(1, iwc + 1), ierr);
    if (ierr != 0) {
      goto statement_34;
    }
  }
  //C
  //C Successful termination.
  //C
  ier = 0;
  return;
  //C
  //C IN1 and IN2 were adjacent on input.
  //C
  statement_30:
  ier = 0;
  return;
  //C
  //C Invalid input parameter.
  //C
  statement_31:
  ier = 1;
  return;
  //C
  //C Insufficient space reserved for IWK.
  //C
  statement_32:
  ier = 2;
  return;
  //C
  //C Invalid triangulation data structure or collinear nodes
  //C   on convex hull boundary.
  //C
  statement_33:
  ier = 3;
  //C       WRITE (*,130) IN1, IN2
  return;
  //C
  //C Error flag returned by OPTIM.
  //C
  statement_34:
  ier = 4;
  //C       WRITE (*,140) NIT, IERR
}

//C      ALGORITHM 751, COLLECTED ALGORITHMS FROM ACM.
//C      THIS WORK PUBLISHED IN TRANSACTIONS ON MATHEMATICAL SOFTWARE,
//C      VOL. 22, NO. 1, March, 1996, P.  1--8.
//C      ####### With remark from renka (to appear) 4/dec/1998
//C
//C      modifications for R:
//C        REAL -> DOUBLE PRECISION  albrecht.gebhardt@uni-klu.ac.at
//C
void
addcst(
  common& cmn,
  int const& ncc,
  arr_cref<int> lcc,
  int const& n,
  arr_cref<double> x,
  arr_cref<double> y,
  int& lwk,
  arr_ref<int> iwk,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& ier)
{
  lcc(dimension(star));
  x(dimension(n));
  y(dimension(n));
  iwk(dimension(lwk));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  int lwd2 = fem::int0;
  int lccip1 = fem::int0;
  int i = fem::int0;
  int ifrst = fem::int0;
  int ilast = fem::int0;
  int n1 = fem::int0;
  int n2 = fem::int0;
  int lw = fem::int0;
  int kbak = fem::int0;
  int k = fem::int0;
  int kfor = fem::int0;
  int lpf = fem::int0;
  int lpb = fem::int0;
  int lpl = fem::int0;
  int lp = fem::int0;
  int kn = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   11/12/94
  //C
  //C   This subroutine provides for creation of a constrained
  //C Delaunay triangulation which, in some sense, covers an
  //C arbitrary connected region R rather than the convex hull
  //C of the nodes.  This is achieved simply by forcing the
  //C presence of certain adjacencies (triangulation arcs) cor-
  //C responding to constraint curves.  The union of triangles
  //C coincides with the convex hull of the nodes, but triangles
  //C in R can be distinguished from those outside of R.  The
  //C only modification required to generalize the definition of
  //C the Delaunay triangulation is replacement of property 5
  //C (refer to TRMESH) by the following:
  //C
  //C  5')  If a node is contained in the interior of the cir-
  //C       cumcircle of a triangle, then every interior point
  //C       of the triangle is separated from the node by a
  //C       constraint arc.
  //C
  //C   In order to be explicit, we make the following defini-
  //C tions.  A constraint region is the open interior of a
  //C simple closed positively oriented polygonal curve defined
  //C by an ordered sequence of three or more distinct nodes
  //C (constraint nodes) P(1),P(2),...,P(K), such that P(I) is
  //C adjacent to P(I+1) for I = 1,...,K with P(K+1) = P(1).
  //C Thus, the constraint region is on the left (and may have
  //C nonfinite area) as the sequence of constraint nodes is
  //C traversed in the specified order.  The constraint regions
  //C must not contain nodes and must not overlap.  The region
  //C R is the convex hull of the nodes with constraint regions
  //C excluded.
  //C
  //C   Note that the terms boundary node and boundary arc are
  //C reserved for nodes and arcs on the boundary of the convex
  //C hull of the nodes.
  //C
  //C   The algorithm is as follows:  given a triangulation
  //C which includes one or more sets of constraint nodes, the
  //C corresponding adjacencies (constraint arcs) are forced to
  //C be present (Subroutine EDGE).  Any additional new arcs
  //C required are chosen to be locally optimal (satisfy the
  //C modified circumcircle property).
  //C
  //C On input:
  //C
  //C       NCC = Number of constraint curves (constraint re-
  //C             gions).  NCC .GE. 0.
  //C
  //C       LCC = Array of length NCC (or dummy array of length
  //C             1 if NCC = 0) containing the index (for X, Y,
  //C             and LEND) of the first node of constraint I in
  //C             LCC(I) for I = 1 to NCC.  Thus, constraint I
  //C             contains K = LCC(I+1) - LCC(I) nodes, K .GE.
  //C             3, stored in (X,Y) locations LCC(I), ...,
  //C             LCC(I+1)-1, where LCC(NCC+1) = N+1.
  //C
  //C       N = Number of nodes in the triangulation, including
  //C           constraint nodes.  N .GE. 3.
  //C
  //C       X,Y = Arrays of length N containing the coordinates
  //C             of the nodes with non-constraint nodes in the
  //C             first LCC(1)-1 locations, followed by NCC se-
  //C             quences of constraint nodes.  Only one of
  //C             these sequences may be specified in clockwise
  //C             order to represent an exterior constraint
  //C             curve (a constraint region with nonfinite
  //C             area).
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LWK = Length of IWK.  This must be at least 2*NI
  //C             where NI is the maximum number of arcs which
  //C             intersect a constraint arc to be added.  NI
  //C             is bounded by N-3.
  //C
  //C       IWK = Integer work array of length LWK (used by
  //C             Subroutine EDGE to add constraint arcs).
  //C
  //C       LIST,LPTR,LEND = Data structure defining the trian-
  //C                        gulation.  Refer to Subroutine
  //C                        TRMESH.
  //C
  //C On output:
  //C
  //C       LWK = Required length of IWK unless IER = 1 or IER =
  //C             3.  In the case of IER = 1, LWK is not altered
  //C             from its input value.
  //C
  //C       IWK = Array containing the endpoint indexes of the
  //C             new arcs which were swapped in by the last
  //C             call to Subroutine EDGE.
  //C
  //C       LIST,LPTR,LEND = Triangulation data structure with
  //C                        all constraint arcs present unless
  //C                        IER .NE. 0.  These arrays are not
  //C                        altered if IER = 1.
  //C
  //C       IER = Error indicator:
  //C             IER = 0 if no errors were encountered.
  //C             IER = 1 if NCC, N, or an LCC entry is outside
  //C                     its valid range, or LWK .LT. 0 on
  //C                     input.
  //C             IER = 2 if more space is required in IWK.
  //C             IER = 3 if the triangulation data structure is
  //C                     invalid, or failure (in EDGE or OPTIM)
  //C                     was caused by collinear nodes on the
  //C                     convex hull boundary.  An error mes-
  //C                     sage is written to logical unit 6 in
  //C                     this case.
  //C             IER = 4 if intersecting constraint arcs were
  //C                     encountered.
  //C             IER = 5 if a constraint region contains a
  //C                     node.
  //C
  //C Modules required by ADDCST:  EDGE, LEFT, LSTPTR, OPTIM,
  //C                                SWAP, SWPTST
  //C
  //C Intrinsic functions called by ADDCST:  ABS, MAX
  //C
  //C***********************************************************
  //C
  lwd2 = lwk / 2;
  //C
  //C Test for errors in input parameters.
  //C
  ier = 1;
  if (ncc < 0 || lwk < 0) {
    return;
  }
  if (ncc == 0) {
    if (n < 3) {
      return;
    }
    lwk = 0;
    goto statement_9;
  }
  else {
    lccip1 = n + 1;
    FEM_DOSTEP(i, ncc, 1, -1) {
      if (lccip1 - lcc(i) < 3) {
        return;
      }
      lccip1 = lcc(i);
    }
    if (lccip1 < 1) {
      return;
    }
  }
  //C
  //C Force the presence of constraint arcs.  The outer loop is
  //C   on constraints in reverse order.  IFRST and ILAST are
  //C   the first and last nodes of constraint I.
  //C
  lwk = 0;
  ifrst = n + 1;
  FEM_DOSTEP(i, ncc, 1, -1) {
    ilast = ifrst - 1;
    ifrst = lcc(i);
    //C
    //C   Inner loop on constraint arcs N1-N2 in constraint I.
    //C
    n1 = ilast;
    FEM_DO_SAFE(n2, ifrst, ilast) {
      lw = lwd2;
      edge(cmn, n1, n2, x, y, lw, iwk, list, lptr, lend, ier);
      lwk = fem::max(lwk, 2 * lw);
      if (ier == 4) {
        ier = 3;
      }
      if (ier != 0) {
        return;
      }
      n1 = n2;
    }
  }
  //C
  //C Test for errors.  The outer loop is on constraint I with
  //C   first and last nodes IFRST and ILAST, and the inner loop
  //C   is on constraint nodes K with (KBAK,K,KFOR) a subse-
  //C   quence of constraint I.
  //C
  ier = 4;
  ifrst = n + 1;
  FEM_DOSTEP(i, ncc, 1, -1) {
    ilast = ifrst - 1;
    ifrst = lcc(i);
    kbak = ilast;
    FEM_DO_SAFE(k, ifrst, ilast) {
      kfor = k + 1;
      if (k == ilast) {
        kfor = ifrst;
      }
      //C
      //C   Find the LIST pointers LPF and LPB of KFOR and KBAK as
      //C     neighbors of K.
      //C
      lpf = 0;
      lpb = 0;
      lpl = lend(k);
      lp = lpl;
      //C
      statement_4:
      lp = lptr(lp);
      kn = fem::abs(list(lp));
      if (kn == kfor) {
        lpf = lp;
      }
      if (kn == kbak) {
        lpb = lp;
      }
      if (lp != lpl) {
        goto statement_4;
      }
      //C
      //C   A pair of intersecting constraint arcs was encountered
      //C     if and only if a constraint arc is missing (introduc-
      //C     tion of the second caused the first to be swapped out).
      //C
      if (lpf == 0 || lpb == 0) {
        return;
      }
      //C
      //C   Loop on neighbors KN of node K which follow KFOR and
      //C     precede KBAK.  The constraint region contains no nodes
      //C     if and only if all such nodes KN are in constraint I.
      //C
      lp = lpf;
      statement_5:
      lp = lptr(lp);
      if (lp == lpb) {
        goto statement_6;
      }
      kn = fem::abs(list(lp));
      if (kn < ifrst || kn > ilast) {
        goto statement_10;
      }
      goto statement_5;
      //C
      //C   Bottom of loop.
      //C
      statement_6:
      kbak = k;
    }
  }
  //C
  //C No errors encountered.
  //C
  statement_9:
  ier = 0;
  return;
  //C
  //C A constraint region contains a node.
  //C
  statement_10:
  ier = 5;
}

double
areap(
  arr_cref<double> x,
  arr_cref<double> y,
  int const& nb,
  arr_cref<int> nodes)
{
  double return_value = fem::double0;
  x(dimension(star));
  y(dimension(star));
  nodes(dimension(nb));
  int nnb = fem::int0;
  double a = fem::double0;
  int nd2 = fem::int0;
  int i = fem::int0;
  int nd1 = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   09/21/90
  //C
  //C   Given a sequence of NB points in the plane, this func-
  //C tion computes the signed area bounded by the closed poly-
  //C gonal curve which passes through the points in the
  //C specified order.  Each simple closed curve is positively
  //C oriented (bounds positive area) if and only if the points
  //C are specified in counterclockwise order.  The last point
  //C of the curve is taken to be the first point specified, and
  //C this point should therefore not be specified twice.
  //C
  //C   The area of a triangulation may be computed by calling
  //C AREAP with values of NB and NODES determined by Subroutine
  //C BNODES.
  //C
  //C On input:
  //C
  //C       X,Y = Arrays of length N containing the Cartesian
  //C             coordinates of a set of points in the plane
  //C             for some N .GE. NB.
  //C
  //C       NB = Length of NODES.
  //C
  //C       NODES = Array of length NB containing the ordered
  //C               sequence of nodal indexes (in the range
  //C               1 to N) which define the polygonal curve.
  //C
  //C Input parameters are not altered by this function.
  //C
  //C On output:
  //C
  //C       AREAP = Signed area bounded by the polygonal curve,
  //C              or zero if NB < 3.
  //C
  //C Modules required by AREAP:  None
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C A =       Partial sum of signed (and doubled) trapezoid
  //C             areas
  //C I =       DO-loop and NODES index
  //C ND1,ND2 = Elements of NODES
  //C NNB =     Local copy of NB
  //C
  nnb = nb;
  a = 0.f;
  if (nnb < 3) {
    goto statement_2;
  }
  nd2 = nodes(nnb);
  //C
  //C Loop on line segments NODES(I-1) -> NODES(I), where
  //C   NODES(0) = NODES(NB), adding twice the signed trapezoid
  //C   areas (integrals of the linear interpolants) to A.
  //C
  FEM_DO_SAFE(i, 1, nnb) {
    nd1 = nd2;
    nd2 = nodes(i);
    a += (x(nd2) - x(nd1)) * (y(nd1) + y(nd2));
  }
  //C
  //C A contains twice the negative signed area of the region.
  //C
  statement_2:
  return_value = -a / 2.f;
  return return_value;
}

void
bnodes(
  int const& n,
  arr_cref<int> list,
  arr_cref<int> lptr,
  arr_cref<int> lend,
  arr_ref<int> nodes,
  int& nb,
  int& na,
  int& nt)
{
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  nodes(dimension(star));
  int nst = fem::int0;
  int lp = fem::int0;
  int k = fem::int0;
  int n0 = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   09/01/88
  //C
  //C   Given a triangulation of N points in the plane, this
  //C subroutine returns an array containing the indexes, in
  //C counterclockwise order, of the nodes on the boundary of
  //C the convex hull of the set of points.
  //C
  //C On input:
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 3.
  //C
  //C       LIST,LPTR,LEND = Data structure defining the trian-
  //C                        gulation.  Refer to Subroutine
  //C                        TRMESH.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       NODES = Integer array of length at least NB
  //C               (NB .LE. N).
  //C
  //C On output:
  //C
  //C       NODES = Ordered sequence of boundary node indexes
  //C               in the range 1 to N.
  //C
  //C       NB = Number of boundary nodes.
  //C
  //C       NA,NT = Number of arcs and triangles, respectively,
  //C               in the triangulation.
  //C
  //C Modules required by BNODES:  None
  //C
  //C***********************************************************
  //C
  //C Set NST to the first boundary node encountered.
  //C
  nst = 1;
  statement_1:
  lp = lend(nst);
  if (list(lp) < 0) {
    goto statement_2;
  }
  nst++;
  goto statement_1;
  //C
  //C Initialization.
  //C
  statement_2:
  nodes(1) = nst;
  k = 1;
  n0 = nst;
  //C
  //C Traverse the boundary in counterclockwise order.
  //C
  statement_3:
  lp = lend(n0);
  lp = lptr(lp);
  n0 = list(lp);
  if (n0 == nst) {
    goto statement_4;
  }
  k++;
  nodes(k) = n0;
  goto statement_3;
  //C
  //C Termination.
  //C
  statement_4:
  nb = k;
  nt = 2 * n - nb - 2;
  na = nt + n - 1;
}

void
circum(
  double const& x1,
  double const& y1,
  double const& x2,
  double const& y2,
  double const& x3,
  double const& y3,
  bool const& ratio,
  double& xc,
  double& yc,
  double& cr,
  double& sa,
  double& ar)
{
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   12/10/96
  //C
  //C   Given three vertices defining a triangle, this subrou-
  //C tine returns the circumcenter, circumradius, signed
  //C triangle area, and, optionally, the aspect ratio of the
  //C triangle.
  //C
  //C On input:
  //C
  //C       X1,...,Y3 = Cartesian coordinates of the vertices.
  //C
  //C       RATIO = Logical variable with value TRUE if and only
  //C               if the aspect ratio is to be computed.
  //C
  //C Input parameters are not altered by this routine.
  //C
  //C On output:
  //C
  //C       XC,YC = Cartesian coordinates of the circumcenter
  //C               (center of the circle defined by the three
  //C               points) unless SA = 0, in which XC and YC
  //C               are not altered.
  //C
  //C       CR = Circumradius (radius of the circle defined by
  //C            the three points) unless SA = 0 (infinite
  //C            radius), in which case CR is not altered.
  //C
  //C       SA = Signed triangle area with positive value if
  //C            and only if the vertices are specified in
  //C            counterclockwise order:  (X3,Y3) is strictly
  //C            to the left of the directed line from (X1,Y1)
  //C            toward (X2,Y2).
  //C
  //C       AR = Aspect ratio r/CR, where r is the radius of the
  //C            inscribed circle, unless RATIO = FALSE, in
  //C            which case AR is not altered.  AR is in the
  //C            range 0 to .5, with value 0 iff SA = 0 and
  //C            value .5 iff the vertices define an equilateral
  //C            triangle.
  //C
  //C Modules required by CIRCUM:  None
  //C
  //C Intrinsic functions called by CIRCUM:  ABS, SQRT
  //C
  //C***********************************************************
  //C
  //C Set U(K) and V(K) to the x and y components, respectively,
  //C   of the directed edge opposite vertex K.
  //C
  arr_1d<3, double> u(fem::fill0);
  u(1) = x3 - x2;
  u(2) = x1 - x3;
  u(3) = x2 - x1;
  arr_1d<3, double> v(fem::fill0);
  v(1) = y3 - y2;
  v(2) = y1 - y3;
  v(3) = y2 - y1;
  //C
  //C Set SA to the signed triangle area.
  //C
  sa = (u(1) * v(2) - u(2) * v(1)) / 2.f;
  if (sa == 0.f) {
    if (ratio) {
      ar = 0.f;
    }
    return;
  }
  //C
  //C Set DS(K) to the squared distance from the origin to
  //C   vertex K.
  //C
  arr_1d<3, double> ds(fem::fill0);
  ds(1) = x1 * x1 + y1 * y1;
  ds(2) = x2 * x2 + y2 * y2;
  ds(3) = x3 * x3 + y3 * y3;
  //C
  //C Compute factors of XC and YC.
  //C
  double fx = 0.f;
  double fy = 0.f;
  int i = fem::int0;
  FEM_DO_SAFE(i, 1, 3) {
    fx = fx - ds(i) * v(i);
    fy += ds(i) * u(i);
  }
  xc = fx / (4.f * sa);
  yc = fy / (4.f * sa);
  cr = fem::sqrt(fem::pow2((xc - x1)) + fem::pow2((yc - y1)));
  if (!ratio) {
    return;
  }
  //C
  //C Compute the squared edge lengths and aspect ratio.
  //C
  FEM_DO_SAFE(i, 1, 3) {
    ds(i) = u(i) * u(i) + v(i) * v(i);
  }
  ar = 2.f * fem::abs(sa) / ((fem::sqrt(ds(1)) + fem::sqrt(ds(2)) +
    fem::sqrt(ds(3))) * cr);
}

void
delnb(
  int const& n0,
  int const& nb,
  int const& n,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& lnew,
  int& lph)
{
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  int nn = fem::int0;
  int lpl = fem::int0;
  int lpp = fem::int0;
  int lpb = fem::int0;
  int lp = fem::int0;
  int lnw = fem::int0;
  int i = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   07/30/98
  //C
  //C   This subroutine deletes a neighbor NB from the adjacency
  //C list of node N0 (but N0 is not deleted from the adjacency
  //C list of NB) and, if NB is a boundary node, makes N0 a
  //C boundary node.  For pointer (LIST index) LPH to NB as a
  //C neighbor of N0, the empty LIST,LPTR location LPH is filled
  //C in with the values at LNEW-1, pointer LNEW-1 (in LPTR and
  //C possibly in LEND) is changed to LPH, and LNEW is decremen-
  //C ted.  This requires a search of LEND and LPTR entailing an
  //C expected operation count of O(N).
  //C
  //C On input:
  //C
  //C       N0,NB = Indexes, in the range 1 to N, of a pair of
  //C               nodes such that NB is a neighbor of N0.
  //C               (N0 need not be a neighbor of NB.)
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 3.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure defining the
  //C                             triangulation.
  //C
  //C On output:
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure updated with
  //C                             the removal of NB from the ad-
  //C                             jacency list of N0 unless
  //C                             LPH < 0.
  //C
  //C       LPH = List pointer to the hole (NB as a neighbor of
  //C             N0) filled in by the values at LNEW-1 or error
  //C             indicator:
  //C             LPH > 0 if no errors were encountered.
  //C             LPH = -1 if N0, NB, or N is outside its valid
  //C                      range.
  //C             LPH = -2 if NB is not a neighbor of N0.
  //C
  //C Modules required by DELNB:  None
  //C
  //C Intrinsic function called by DELNB:  ABS
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C I =   DO-loop index
  //C LNW = LNEW-1 (output value of LNEW)
  //C LP =  LIST pointer of the last neighbor of NB
  //C LPB = Pointer to NB as a neighbor of N0
  //C LPL = Pointer to the last neighbor of N0
  //C LPP = Pointer to the neighbor of N0 that precedes NB
  //C NN =  Local copy of N
  //C
  nn = n;
  //C
  //C Test for error 1.
  //C
  if (n0 < 1 || n0 > nn || nb < 1 || nb > nn || nn < 3) {
    lph = -1;
    return;
  }
  //C
  //C   Find pointers to neighbors of N0:
  //C
  //C     LPL points to the last neighbor,
  //C     LPP points to the neighbor NP preceding NB, and
  //C     LPB points to NB.
  //C
  lpl = lend(n0);
  lpp = lpl;
  lpb = lptr(lpp);
  statement_1:
  if (list(lpb) == nb) {
    goto statement_2;
  }
  lpp = lpb;
  lpb = lptr(lpp);
  if (lpb != lpl) {
    goto statement_1;
  }
  //C
  //C   Test for error 2 (NB not found).
  //C
  if (fem::abs(list(lpb)) != nb) {
    lph = -2;
    return;
  }
  //C
  //C   NB is the last neighbor of N0.  Make NP the new last
  //C     neighbor and, if NB is a boundary node, then make N0
  //C     a boundary node.
  //C
  lend(n0) = lpp;
  lp = lend(nb);
  if (list(lp) < 0) {
    list(lpp) = -list(lpp);
  }
  goto statement_3;
  //C
  //C   NB is not the last neighbor of N0.  If NB is a boundary
  //C     node and N0 is not, then make N0 a boundary node with
  //C     last neighbor NP.
  //C
  statement_2:
  lp = lend(nb);
  if (list(lp) < 0 && list(lpl) > 0) {
    lend(n0) = lpp;
    list(lpp) = -list(lpp);
  }
  //C
  //C   Update LPTR so that the neighbor following NB now fol-
  //C     lows NP, and fill in the hole at location LPB.
  //C
  statement_3:
  lptr(lpp) = lptr(lpb);
  lnw = lnew - 1;
  list(lpb) = list(lnw);
  lptr(lpb) = lptr(lnw);
  FEM_DOSTEP(i, nn, 1, -1) {
    if (lend(i) == lnw) {
      lend(i) = lpb;
      goto statement_5;
    }
  }
  //C
  statement_5:
  FEM_DO_SAFE(i, 1, lnw - 1) {
    if (lptr(i) == lnw) {
      lptr(i) = lpb;
    }
  }
  //C
  //C No errors encountered.
  //C
  lnew = lnw;
  lph = lpb;
}

void
delarc(
  int const& n,
  int const& io1,
  int const& io2,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& lnew,
  int& ier)
{
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   11/12/94
  //C
  //C   This subroutine deletes a boundary arc from a triangula-
  //C tion.  It may be used to remove a null triangle from the
  //C convex hull boundary.  Note, however, that if the union of
  //C triangles is rendered nonconvex, Subroutines DELNOD, EDGE,
  //C and TRFIND may fail.  Thus, Subroutines ADDCST, ADDNOD,
  //C DELNOD, EDGE, and NEARND should not be called following
  //C an arc deletion.
  //C
  //C On input:
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 4.
  //C
  //C       IO1,IO2 = Indexes (in the range 1 to N) of a pair of
  //C                 adjacent boundary nodes defining the arc
  //C                 to be removed.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LIST,LPTR,LEND,LNEW = Triangulation data structure
  //C                             created by TRMESH or TRMSHR.
  //C
  //C On output:
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure updated with
  //C                             the removal of arc IO1-IO2
  //C                             unless IER > 0.
  //C
  //C       IER = Error indicator:
  //C             IER = 0 if no errors were encountered.
  //C             IER = 1 if N, IO1, or IO2 is outside its valid
  //C                     range, or IO1 = IO2.
  //C             IER = 2 if IO1-IO2 is not a boundary arc.
  //C             IER = 3 if the node opposite IO1-IO2 is al-
  //C                     ready a boundary node, and thus IO1
  //C                     or IO2 has only two neighbors or a
  //C                     deletion would result in two triangu-
  //C                     lations sharing a single node.
  //C             IER = 4 if one of the nodes is a neighbor of
  //C                     the other, but not vice versa, imply-
  //C                     ing an invalid triangulation data
  //C                     structure.
  //C
  //C Modules required by DELARC:  DELNB, LSTPTR
  //C
  //C Intrinsic function called by DELARC:  ABS
  //C
  //C***********************************************************
  //C
  int n1 = io1;
  int n2 = io2;
  //C
  //C Test for errors, and set N1->N2 to the directed boundary
  //C   edge associated with IO1-IO2:  (N1,N2,N3) is a triangle
  //C   for some N3.
  //C
  if (n < 4 || n1 < 1 || n1 > n || n2 < 1 || n2 > n || n1 == n2) {
    ier = 1;
    return;
  }
  //C
  int lpl = lend(n2);
  if (-list(lpl) != n1) {
    n1 = n2;
    n2 = io1;
    lpl = lend(n2);
    if (-list(lpl) != n1) {
      ier = 2;
      return;
    }
  }
  //C
  //C Set N3 to the node opposite N1->N2 (the second neighbor
  //C   of N1), and test for error 3 (N3 already a boundary
  //C   node).
  //C
  lpl = lend(n1);
  int lp = lptr(lpl);
  lp = lptr(lp);
  int n3 = fem::abs(list(lp));
  lpl = lend(n3);
  if (list(lpl) <= 0) {
    ier = 3;
    return;
  }
  //C
  //C Delete N2 as a neighbor of N1, making N3 the first
  //C   neighbor, and test for error 4 (N2 not a neighbor
  //C   of N1).  Note that previously computed pointers may
  //C   no longer be valid following the call to DELNB.
  //C
  int lph = fem::int0;
  delnb(n1, n2, n, list, lptr, lend, lnew, lph);
  if (lph < 0) {
    ier = 4;
    return;
  }
  //C
  //C Delete N1 as a neighbor of N2, making N3 the new last
  //C   neighbor.
  //C
  delnb(n2, n1, n, list, lptr, lend, lnew, lph);
  //C
  //C Make N3 a boundary node with first neighbor N2 and last
  //C   neighbor N1.
  //C
  lp = lstptr(lend(n3), n1, list, lptr);
  lend(n3) = lp;
  list(lp) = -n1;
  //C
  //C No errors encountered.
  //C
  ier = 0;
}

int
nbcnt(
  int const& lpl,
  arr_cref<int> lptr)
{
  int return_value = fem::int0;
  lptr(dimension(star));
  int lp = fem::int0;
  int k = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   09/01/88
  //C
  //C   This function returns the number of neighbors of a node
  //C N0 in a triangulation created by Subroutine TRMESH (or
  //C TRMSHR).
  //C
  //C On input:
  //C
  //C       LPL = LIST pointer to the last neighbor of N0 --
  //C             LPL = LEND(N0).
  //C
  //C       LPTR = Array of pointers associated with LIST.
  //C
  //C Input parameters are not altered by this function.
  //C
  //C On output:
  //C
  //C       NBCNT = Number of neighbors of N0.
  //C
  //C Modules required by NBCNT:  None
  //C
  //C***********************************************************
  //C
  lp = lpl;
  k = 1;
  //C
  statement_1:
  lp = lptr(lp);
  if (lp == lpl) {
    goto statement_2;
  }
  k++;
  goto statement_1;
  //C
  statement_2:
  return_value = k;
  return return_value;
}

void
delnod(
  common& cmn,
  int const& k,
  int const& ncc,
  arr_ref<int> lcc,
  int& n,
  arr_ref<double> x,
  arr_ref<double> y,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& lnew,
  int& lwk,
  arr_ref<int, 2> iwk,
  int& ier)
{
  lcc(dimension(star));
  x(dimension(star));
  y(dimension(star));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(star));
  iwk(dimension(2, star));
  int n1 = fem::int0;
  int nn = fem::int0;
  int lccip1 = fem::int0;
  int i = fem::int0;
  int lpl = fem::int0;
  int lpf = fem::int0;
  int nnb = fem::int0;
  bool bdry = fem::bool0;
  int lwkl = fem::int0;
  int iwl = fem::int0;
  double x1 = fem::double0;
  double y1 = fem::double0;
  int nfrst = fem::int0;
  int nr = fem::int0;
  double xr = fem::double0;
  double yr = fem::double0;
  int lp = fem::int0;
  int n2 = fem::int0;
  double x2 = fem::double0;
  double y2 = fem::double0;
  int nl = fem::int0;
  double xl = fem::double0;
  double yl = fem::double0;
  int lpl2 = fem::int0;
  int lp21 = fem::int0;
  int lnw = fem::int0;
  int lph = fem::int0;
  int lpn = fem::int0;
  int j = fem::int0;
  int nit = fem::int0;
  int ierr = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   06/28/98
  //C
  //C   This subroutine deletes node K (along with all arcs
  //C incident on node K) from a triangulation of N nodes in the
  //C plane, and inserts arcs as necessary to produce a triangu-
  //C lation of the remaining N-1 nodes.  If a Delaunay triangu-
  //C lation is input, a Delaunay triangulation will result, and
  //C thus, DELNOD reverses the effect of a call to Subroutine
  //C ADDNOD.
  //C
  //C   Note that a constraint node cannot be deleted by this
  //C routine.  In order to delete a constraint node, it is
  //C necessary to call this routine with NCC = 0, decrement the
  //C appropriate LCC entries (LCC(I) such that LCC(I) > K), and
  //C then create (or restore) the constraints by a call to Sub-
  //C routine ADDCST.
  //C
  //C On input:
  //C
  //C       K = Index (for X and Y) of the node to be deleted.
  //C           1 .LE. K .LT. LCC(1).  (K .LE. N if NCC=0).
  //C
  //C       NCC = Number of constraint curves.  NCC .GE. 0.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       LCC = List of constraint curve starting indexes (or
  //C             dummy array of length 1 if NCC = 0).  Refer to
  //C             Subroutine ADDCST.
  //C
  //C       N = Number of nodes in the triangulation on input.
  //C           N .GE. 4.  Note that N will be decremented
  //C           following the deletion.
  //C
  //C       X,Y = Arrays of length N containing the coordinates
  //C             of the nodes with non-constraint nodes in the
  //C             first LCC(1)-1 locations if NCC > 0.
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure defining the
  //C                             triangulation.  Refer to Sub-
  //C                             routine TRMESH.
  //C
  //C       LWK = Number of columns reserved for IWK.  LWK must
  //C             be at least NNB-3, where NNB is the number of
  //C             neighbors of node K, including an extra
  //C             pseudo-node if K is a boundary node.
  //C
  //C       IWK = Integer work array dimensioned 2 by LWK (or
  //C             array of length .GE. 2*LWK).
  //C
  //C On output:
  //C
  //C       LCC = List of constraint curve starting indexes de-
  //C             cremented by 1 to reflect the deletion of K
  //C             unless NCC = 0 or 1 .LE. IER .LE. 4.
  //C
  //C       N = New number of nodes (input value minus one) un-
  //C           less 1 .LE. IER .LE. 4.
  //C
  //C       X,Y = Updated arrays of length N-1 containing nodal
  //C             coordinates (with elements K+1,...,N shifted
  //C             up a position and thus overwriting element K)
  //C             unless 1 .LE. IER .LE. 4.  (N here denotes the
  //C             input value.)
  //C
  //C       LIST,LPTR,LEND,LNEW = Updated triangulation data
  //C                             structure reflecting the dele-
  //C                             tion unless IER .NE. 0.  Note
  //C                             that the data structure may
  //C                             have been altered if IER .GE.
  //C                             3.
  //C
  //C       LWK = Number of IWK columns required unless IER = 1
  //C             or IER = 3.
  //C
  //C       IWK = Indexes of the endpoints of the new arcs added
  //C             unless LWK = 0 or 1 .LE. IER .LE. 4.  (Arcs
  //C             are associated with columns, or pairs of
  //C             adjacent elements if IWK is declared as a
  //C             singly-subscripted array.)
  //C
  //C       IER = Error indicator:
  //C             IER = 0 if no errors were encountered.
  //C             IER = 1 if K, NCC, N, or an LCC entry is out-
  //C                     side its valid range or LWK < 0 on
  //C                     input.
  //C             IER = 2 if more space is required in IWK.
  //C                     Refer to LWK.
  //C             IER = 3 if the triangulation data structure is
  //C                     invalid on input.
  //C             IER = 4 if K is an interior node with 4 or
  //C                     more neighbors, and the number of
  //C                     neighbors could not be reduced to 3
  //C                     by swaps.  This could be caused by
  //C                     floating point errors with collinear
  //C                     nodes or by an invalid data structure.
  //C             IER = 5 if an error flag was returned by
  //C                     OPTIM.  An error message is written
  //C                     to the standard output unit in this
  //C                     event.
  //C
  //C   Note that the deletion may result in all remaining nodes
  //C being collinear.  This situation is not flagged.
  //C
  //C Modules required by DELNOD:  DELNB, LEFT, LSTPTR, NBCNT,
  //C                                OPTIM, SWAP, SWPTST
  //C
  //C Intrinsic function called by DELNOD:  ABS
  //C
  //C***********************************************************
  //C
  //C Set N1 to K and NNB to the number of neighbors of N1 (plus
  //C   one if N1 is a boundary node), and test for errors.  LPF
  //C   and LPL are LIST indexes of the first and last neighbors
  //C   of N1, IWL is the number of IWK columns containing arcs,
  //C   and BDRY is TRUE iff N1 is a boundary node.
  //C
  n1 = k;
  nn = n;
  if (ncc < 0 || n1 < 1 || nn < 4 || lwk < 0) {
    goto statement_21;
  }
  lccip1 = nn + 1;
  FEM_DOSTEP(i, ncc, 1, -1) {
    if (lccip1 - lcc(i) < 3) {
      goto statement_21;
    }
    lccip1 = lcc(i);
  }
  if (n1 >= lccip1) {
    goto statement_21;
  }
  lpl = lend(n1);
  lpf = lptr(lpl);
  nnb = nbcnt(lpl, lptr);
  bdry = list(lpl) < 0;
  if (bdry) {
    nnb++;
  }
  if (nnb < 3) {
    goto statement_23;
  }
  lwkl = lwk;
  lwk = nnb - 3;
  if (lwkl < lwk) {
    goto statement_22;
  }
  iwl = 0;
  if (nnb == 3) {
    goto statement_5;
  }
  //C
  //C Initialize for loop on arcs N1-N2 for neighbors N2 of N1,
  //C   beginning with the second neighbor.  NR and NL are the
  //C   neighbors preceding and following N2, respectively, and
  //C   LP indexes NL.  The loop is exited when all possible
  //C   swaps have been applied to arcs incident on N1.  If N1
  //C   is interior, the number of neighbors will be reduced
  //C   to 3.
  //C
  x1 = x(n1);
  y1 = y(n1);
  nfrst = list(lpf);
  nr = nfrst;
  xr = x(nr);
  yr = y(nr);
  lp = lptr(lpf);
  n2 = list(lp);
  x2 = x(n2);
  y2 = y(n2);
  lp = lptr(lp);
  //C
  //C Top of loop:  set NL to the neighbor following N2.
  //C
  statement_2:
  nl = fem::abs(list(lp));
  if (nl == nfrst && bdry) {
    goto statement_5;
  }
  xl = x(nl);
  yl = y(nl);
  //C
  //C   Test for a convex quadrilateral.  To avoid an incorrect
  //C     test caused by collinearity, use the fact that if N1
  //C     is a boundary node, then N1 LEFT NR->NL and if N2 is
  //C     a boundary node, then N2 LEFT NL->NR.
  //C
  lpl2 = lend(n2);
  if ((bdry || left(xr, yr, xl, yl, x1, y1)) && (list(lpl2) < 0 || left(xl,
      yl, xr, yr, x2, y2))) {
    goto statement_3;
  }
  //C
  //C   Nonconvex quadrilateral -- no swap is possible.
  //C
  nr = n2;
  xr = x2;
  yr = y2;
  goto statement_4;
  //C
  //C   The quadrilateral defined by adjacent triangles
  //C     (N1,N2,NL) and (N2,N1,NR) is convex.  Swap in
  //C     NL-NR and store it in IWK.  Indexes larger than N1
  //C     must be decremented since N1 will be deleted from
  //C     X and Y.
  //C
  statement_3:
  swap(nl, nr, n1, n2, list, lptr, lend, lp21);
  iwl++;
  if (nl <= n1) {
    iwk(1, iwl) = nl;
  }
  else {
    iwk(1, iwl) = nl - 1;
  }
  if (nr <= n1) {
    iwk(2, iwl) = nr;
  }
  else {
    iwk(2, iwl) = nr - 1;
  }
  //C
  //C   Recompute the LIST indexes LPL,LP and decrement NNB.
  //C
  lpl = lend(n1);
  nnb = nnb - 1;
  if (nnb == 3) {
    goto statement_5;
  }
  lp = lstptr(lpl, nl, list, lptr);
  if (nr == nfrst) {
    goto statement_4;
  }
  //C
  //C   NR is not the first neighbor of N1.
  //C     Back up and test N1-NR for a swap again:  Set N2 to
  //C     NR and NR to the previous neighbor of N1 -- the
  //C     neighbor of NR which follows N1.  LP21 points to NL
  //C     as a neighbor of NR.
  //C
  n2 = nr;
  x2 = xr;
  y2 = yr;
  lp21 = lptr(lp21);
  lp21 = lptr(lp21);
  nr = fem::abs(list(lp21));
  xr = x(nr);
  yr = y(nr);
  goto statement_2;
  //C
  //C   Bottom of loop -- test for invalid termination.
  //C
  statement_4:
  if (n2 == nfrst) {
    goto statement_24;
  }
  n2 = nl;
  x2 = xl;
  y2 = yl;
  lp = lptr(lp);
  goto statement_2;
  //C
  //C Delete N1 from the adjacency list of N2 for all neighbors
  //C   N2 of N1.  LPL points to the last neighbor of N1.
  //C   LNEW is stored in local variable LNW.
  //C
  statement_5:
  lp = lpl;
  lnw = lnew;
  //C
  //C Loop on neighbors N2 of N1, beginning with the first.
  //C
  statement_6:
  lp = lptr(lp);
  n2 = fem::abs(list(lp));
  delnb(n2, n1, n, list, lptr, lend, lnw, lph);
  if (lph < 0) {
    goto statement_23;
  }
  //C
  //C   LP and LPL may require alteration.
  //C
  if (lpl == lnw) {
    lpl = lph;
  }
  if (lp == lnw) {
    lp = lph;
  }
  if (lp != lpl) {
    goto statement_6;
  }
  //C
  //C Delete N1 from X, Y, and LEND, and remove its adjacency
  //C   list from LIST and LPTR.  LIST entries (nodal indexes)
  //C   which are larger than N1 must be decremented.
  //C
  nn = nn - 1;
  if (n1 > nn) {
    goto statement_9;
  }
  FEM_DO_SAFE(i, n1, nn) {
    x(i) = x(i + 1);
    y(i) = y(i + 1);
    lend(i) = lend(i + 1);
  }
  //C
  FEM_DO_SAFE(i, 1, lnw - 1) {
    if (list(i) > n1) {
      list(i) = list(i) - 1;
    }
    if (list(i) <  - n1) {
      list(i)++;
    }
  }
  //C
  //C   For LPN = first to last neighbors of N1, delete the
  //C     preceding neighbor (indexed by LP).
  //C
  //C   Each empty LIST,LPTR location LP is filled in with the
  //C     values at LNW-1, and LNW is decremented.  All pointers
  //C     (including those in LPTR and LEND) with value LNW-1
  //C     must be changed to LP.
  //C
  //C  LPL points to the last neighbor of N1.
  //C
  statement_9:
  if (bdry) {
    nnb = nnb - 1;
  }
  lpn = lpl;
  FEM_DO_SAFE(j, 1, nnb) {
    lnw = lnw - 1;
    lp = lpn;
    lpn = lptr(lp);
    list(lp) = list(lnw);
    lptr(lp) = lptr(lnw);
    if (lptr(lpn) == lnw) {
      lptr(lpn) = lp;
    }
    if (lpn == lnw) {
      lpn = lp;
    }
    FEM_DOSTEP(i, nn, 1, -1) {
      if (lend(i) == lnw) {
        lend(i) = lp;
        goto statement_11;
      }
    }
    //C
    statement_11:
    FEM_DOSTEP(i, lnw - 1, 1, -1) {
      if (lptr(i) == lnw) {
        lptr(i) = lp;
      }
    }
  }
  //C
  //C Decrement LCC entries.
  //C
  FEM_DO_SAFE(i, 1, ncc) {
    lcc(i) = lcc(i) - 1;
  }
  //C
  //C Update N and LNEW, and optimize the patch of triangles
  //C   containing K (on input) by applying swaps to the arcs
  //C   in IWK.
  //C
  n = nn;
  lnew = lnw;
  if (iwl > 0) {
    nit = 4 * iwl;
    optim(cmn, x, y, iwl, list, lptr, lend, nit, iwk, ierr);
    if (ierr != 0) {
      goto statement_25;
    }
  }
  //C
  //C Successful termination.
  //C
  ier = 0;
  return;
  //C
  //C Invalid input parameter.
  //C
  statement_21:
  ier = 1;
  return;
  //C
  //C Insufficient space reserved for IWK.
  //C
  statement_22:
  ier = 2;
  return;
  //C
  //C Invalid triangulation data structure.  NNB < 3 on input or
  //C   N2 is a neighbor of N1 but N1 is not a neighbor of N2.
  //C
  statement_23:
  ier = 3;
  return;
  //C
  //C K is an interior node with 4 or more neighbors, but the
  //C   number of neighbors could not be reduced.
  //C
  statement_24:
  ier = 4;
  return;
  //C
  //C Error flag returned by OPTIM.
  //C
  statement_25:
  ier = 5;
  //C       WRITE (*,100) NIT, IERR
}

bool
intsec(
  double const& x1,
  double const& y1,
  double const& x2,
  double const& y2,
  double const& x3,
  double const& y3,
  double const& x4,
  double const& y4)
{
  bool return_value = fem::bool0;
  double dx12 = fem::double0;
  double dy12 = fem::double0;
  double dx34 = fem::double0;
  double dy34 = fem::double0;
  double dx31 = fem::double0;
  double dy31 = fem::double0;
  double a = fem::double0;
  double b = fem::double0;
  double d = fem::double0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   09/01/88
  //C
  //C   Given a pair of line segments P1-P2 and P3-P4, this
  //C function returns the value .TRUE. if and only if P1-P2
  //C shares one or more points with P3-P4.  The line segments
  //C include their endpoints, and the four points need not be
  //C distinct.  Thus, either line segment may consist of a
  //C single point, and the segments may meet in a V (which is
  //C treated as an intersection).  Note that an incorrect
  //C decision may result from floating point error if the four
  //C endpoints are nearly collinear.
  //C
  //C On input:
  //C
  //C       X1,Y1 = Coordinates of P1.
  //C
  //C       X2,Y2 = Coordinates of P2.
  //C
  //C       X3,Y3 = Coordinates of P3.
  //C
  //C       X4,Y4 = Coordinates of P4.
  //C
  //C Input parameters are not altered by this function.
  //C
  //C On output:
  //C
  //C       INTSEC = Logical value defined above.
  //C
  //C Modules required by INTSEC:  None
  //C
  //C***********************************************************
  //C
  //C Test for overlap between the smallest rectangles that
  //C   contain the line segments and have sides parallel to
  //C   the axes.
  //C
  if ((x1 < x3 && x1 < x4 && x2 < x3 && x2 < x4) || (x1 > x3 &&
      x1 > x4 && x2 > x3 && x2 > x4) || (y1 < y3 && y1 < y4 &&
      y2 < y3 && y2 < y4) || (y1 > y3 && y1 > y4 && y2 > y3 &&
      y2 > y4)) {
    return_value = false;
    return return_value;
  }
  //C
  //C Compute A = P4-P3 X P1-P3, B = P2-P1 X P1-P3, and
  //C   D = P2-P1 X P4-P3 (Z components).
  //C
  dx12 = x2 - x1;
  dy12 = y2 - y1;
  dx34 = x4 - x3;
  dy34 = y4 - y3;
  dx31 = x1 - x3;
  dy31 = y1 - y3;
  a = dx34 * dy31 - dx31 * dy34;
  b = dx12 * dy31 - dx31 * dy12;
  d = dx12 * dy34 - dx34 * dy12;
  if (d == 0.f) {
    goto statement_1;
  }
  //C
  //C D .NE. 0 and the point of intersection of the lines de-
  //C   fined by the line segments is P = P1 + (A/D)*(P2-P1) =
  //C   P3 + (B/D)*(P4-P3).
  //C
  return_value = a / d >= 0.f && a / d <= 1.f && b / d >= 0.f && b / d <= 1.f;
  return return_value;
  //C
  //C D .EQ. 0 and thus either the line segments are parallel,
  //C   or one (or both) of them is a single point.
  //C
  statement_1:
  return_value = a == 0.f && b == 0.f;
  return return_value;
}

void
getnp(
  int const& ncc,
  arr_cref<int> lcc,
  int const& n,
  arr_cref<double> x,
  arr_cref<double> y,
  arr_cref<int> list,
  arr_cref<int> lptr,
  arr_ref<int> lend,
  int const& l,
  arr_ref<int> npts,
  arr_ref<double> ds,
  int& ier)
{
  lcc(dimension(star));
  x(dimension(n));
  y(dimension(n));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  npts(dimension(l));
  ds(dimension(l));
  int nn = fem::int0;
  int lcc1 = fem::int0;
  int lm1 = fem::int0;
  int i = fem::int0;
  int k = fem::int0;
  int nk = fem::int0;
  int n1 = fem::int0;
  double x1 = fem::double0;
  double y1 = fem::double0;
  bool isw = fem::bool0;
  double dl = fem::double0;
  int km1 = fem::int0;
  double xk = fem::double0;
  double yk = fem::double0;
  int lpkl = fem::int0;
  int nkfor = fem::int0;
  int nkbak = fem::int0;
  bool vis = fem::bool0;
  int ifrst = fem::int0;
  int ilast = fem::int0;
  int lpk = fem::int0;
  int nc = fem::int0;
  double xc = fem::double0;
  double yc = fem::double0;
  double dc = fem::double0;
  int lpcl = fem::int0;
  int j = fem::int0;
  int nj = fem::int0;
  int lp = fem::int0;
  double xj = fem::double0;
  double yj = fem::double0;
  int nf1 = fem::int0;
  bool ncf = fem::bool0;
  bool njf = fem::bool0;
  bool skip = fem::bool0;
  int nf2 = fem::int0;
  bool sksav = fem::bool0;
  bool lft1 = fem::bool0;
  bool lft2 = fem::bool0;
  bool lft12 = fem::bool0;
  int nl = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   11/12/94
  //C
  //C   Given a triangulation of N nodes and an array NPTS con-
  //C taining the indexes of L-1 nodes ordered by distance from
  //C NPTS(1), this subroutine sets NPTS(L) to the index of the
  //C next node in the sequence -- the node, other than NPTS(1),
  //C ...,NPTS(L-1), which is closest to NPTS(1).  Thus, the
  //C ordered sequence of K closest nodes to N1 (including N1)
  //C may be determined by K-1 calls to GETNP with NPTS(1) = N1
  //C and L = 2,3,...,K for K .GE. 2.  Note that NPTS must in-
  //C clude constraint nodes as well as non-constraint nodes.
  //C Thus, a sequence of K1 closest non-constraint nodes to N1
  //C must be obtained as a subset of the closest K2 nodes to N1
  //C for some K2 .GE. K1.
  //C
  //C   The terms closest and distance have special definitions
  //C when constraint nodes are present in the triangulation.
  //C Nodes N1 and N2 are said to be visible from each other if
  //C and only if the line segment N1-N2 intersects no con-
  //C straint arc (except possibly itself) and is not an interi-
  //C or constraint arc (arc whose interior lies in a constraint
  //C region).  A path from N1 to N2 is an ordered sequence of
  //C nodes, with N1 first and N2 last, such that adjacent path
  //C elements are visible from each other.  The path length is
  //C the sum of the Euclidean distances between adjacent path
  //C nodes.  Finally, the distance from N1 to N2 is defined to
  //C be the length of the shortest path from N1 to N2.
  //C
  //C   The algorithm uses the property of a Delaunay triangula-
  //C tion that the K-th closest node to N1 is a neighbor of one
  //C of the K-1 closest nodes to N1.  With the definition of
  //C distance used here, this property holds when constraints
  //C are present as long as non-constraint arcs are locally
  //C optimal.
  //C
  //C On input:
  //C
  //C       NCC = Number of constraints.  NCC .GE. 0.
  //C
  //C       LCC = List of constraint curve starting indexes (or
  //C             dummy array of length 1 if NCC = 0).  Refer to
  //C             Subroutine ADDCST.
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 3.
  //C
  //C       X,Y = Arrays of length N containing the coordinates
  //C             of the nodes with non-constraint nodes in the
  //C             first LCC(1)-1 locations if NCC > 0.
  //C
  //C       LIST,LPTR,LEND = Triangulation data structure.  Re-
  //C                        fer to Subroutine TRMESH.
  //C
  //C       L = Number of nodes in the sequence on output.  2
  //C           .LE. L .LE. N.
  //C
  //C       NPTS = Array of length .GE. L containing the indexes
  //C              of the L-1 closest nodes to NPTS(1) in the
  //C              first L-1 locations.
  //C
  //C       DS = Array of length .GE. L containing the distance
  //C            (defined above) between NPTS(1) and NPTS(I) in
  //C            the I-th position for I = 1,...,L-1.  Thus,
  //C            DS(1) = 0.
  //C
  //C Input parameters other than NPTS(L) and DS(L) are not
  //C   altered by this routine.
  //C
  //C On output:
  //C
  //C       NPTS = Array updated with the index of the L-th
  //C              closest node to NPTS(1) in position L unless
  //C              IER .NE. 0.
  //C
  //C       DS = Array updated with the distance between NPTS(1)
  //C            and NPTS(L) in position L unless IER .NE. 0.
  //C
  //C       IER = Error indicator:
  //C             IER =  0 if no errors were encountered.
  //C             IER = -1 if NCC, N, L, or an LCC entry is
  //C                      outside its valid range on input.
  //C             IER =  K if NPTS(K) is not a valid index in
  //C                      the range 1 to N.
  //C
  //C Module required by GETNP:  INTSEC
  //C
  //C Intrinsic functions called by GETNP:  ABS, MIN, SQRT
  //C
  //C***********************************************************
  //C
  //C Store parameters in local variables and test for errors.
  //C   LCC1 indexes the first constraint node.
  //C
  ier = -1;
  nn = n;
  lcc1 = nn + 1;
  lm1 = l - 1;
  if (ncc < 0 || lm1 < 1 || lm1 >= nn) {
    return;
  }
  if (ncc == 0) {
    if (nn < 3) {
      return;
    }
  }
  else {
    FEM_DOSTEP(i, ncc, 1, -1) {
      if (lcc1 - lcc(i) < 3) {
        return;
      }
      lcc1 = lcc(i);
    }
    if (lcc1 < 1) {
      return;
    }
  }
  //C
  //C Test for an invalid index in NPTS.
  //C
  FEM_DO_SAFE(k, 1, lm1) {
    nk = npts(k);
    if (nk < 1 || nk > nn) {
      ier = k;
      return;
    }
  }
  //C
  //C Store N1 = NPTS(1) and mark the elements of NPTS.
  //C
  n1 = npts(1);
  x1 = x(n1);
  y1 = y(n1);
  FEM_DO_SAFE(k, 1, lm1) {
    nk = npts(k);
    lend(nk) = -lend(nk);
  }
  //C
  //C Candidates NC for NL = NPTS(L) are the unmarked visible
  //C   neighbors of nodes NK in NPTS.  ISW is an initialization
  //C   switch set to .TRUE. when NL and its distance DL from N1
  //C   have been initialized with the first candidate encount-
  //C   ered.
  //C
  isw = false;
  dl = 0.f;
  //C
  //C Loop on marked nodes NK = NPTS(K).  LPKL indexes the last
  //C   neighbor of NK in LIST.
  //C
  FEM_DO_SAFE(k, 1, lm1) {
    km1 = k - 1;
    nk = npts(k);
    xk = x(nk);
    yk = y(nk);
    lpkl = -lend(nk);
    nkfor = 0;
    nkbak = 0;
    vis = true;
    if (nk >= lcc1) {
      //C
      //C   NK is a constraint node.  Set NKFOR and NKBAK to the
      //C     constraint nodes which follow and precede NK.  IFRST
      //C     and ILAST are set to the first and last nodes in the
      //C     constraint containing NK.
      //C
      ifrst = nn + 1;
      FEM_DOSTEP(i, ncc, 1, -1) {
        ilast = ifrst - 1;
        ifrst = lcc(i);
        if (nk >= ifrst) {
          goto statement_5;
        }
      }
      //C
      statement_5:
      if (nk < ilast) {
        nkfor = nk + 1;
      }
      else {
        nkfor = ifrst;
      }
      if (nk > ifrst) {
        nkbak = nk - 1;
      }
      else {
        nkbak = ilast;
      }
      //C
      //C   Initialize VIS to TRUE iff NKFOR precedes NKBAK in the
      //C     adjacency list for NK -- the first neighbor is visi-
      //C     ble and is not NKBAK.
      //C
      lpk = lpkl;
      statement_6:
      lpk = lptr(lpk);
      nc = fem::abs(list(lpk));
      if (nc != nkfor && nc != nkbak) {
        goto statement_6;
      }
      vis = nc == nkfor;
    }
    //C
    //C Loop on neighbors NC of NK, bypassing marked and nonvis-
    //C   ible neighbors.
    //C
    lpk = lpkl;
    statement_7:
    lpk = lptr(lpk);
    nc = fem::abs(list(lpk));
    if (nc == nkbak) {
      vis = true;
    }
    //C
    //C   VIS = .FALSE. iff NK-NC is an interior constraint arc
    //C     (NK is a constraint node and NC lies strictly between
    //C     NKFOR and NKBAK).
    //C
    if (!vis) {
      goto statement_15;
    }
    if (nc == nkfor) {
      vis = false;
    }
    if (lend(nc) < 0) {
      goto statement_15;
    }
    //C
    //C Initialize distance DC between N1 and NC to Euclidean
    //C   distance.
    //C
    xc = x(nc);
    yc = y(nc);
    dc = fem::sqrt((xc - x1) * (xc - x1) + (yc - y1) * (yc - y1));
    if (isw && dc >= dl) {
      goto statement_15;
    }
    if (k == 1) {
      goto statement_14;
    }
    //C
    //C K .GE. 2.  Store the pointer LPCL to the last neighbor
    //C   of NC.
    //C
    lpcl = lend(nc);
    //C
    //C Set DC to the length of the shortest path from N1 to NC
    //C   which has not previously been encountered and which is
    //C   a viable candidate for the shortest path from N1 to NL.
    //C   This is Euclidean distance iff NC is visible from N1.
    //C   Since the shortest path from N1 to NL contains only ele-
    //C   ments of NPTS which are constraint nodes (in addition to
    //C   N1 and NL), only these need be considered for the path
    //C   from N1 to NC.  Thus, for distance function D(A,B) and
    //C   J = 1,...,K, DC = min(D(N1,NJ) + D(NJ,NC)) over con-
    //C   straint nodes NJ = NPTS(J) which are visible from NC.
    //C
    FEM_DO_SAFE(j, 1, km1) {
      nj = npts(j);
      if (j > 1 && nj < lcc1) {
        goto statement_13;
      }
      //C
      //C If NC is a visible neighbor of NJ, a path from N1 to NC
      //C   containing NJ has already been considered.  Thus, NJ may
      //C   be bypassed if it is adjacent to NC.
      //C
      lp = lpcl;
      statement_8:
      lp = lptr(lp);
      if (nj == fem::abs(list(lp))) {
        goto statement_12;
      }
      if (lp != lpcl) {
        goto statement_8;
      }
      //C
      //C NJ is a constraint node (unless J=1) not adjacent to NC,
      //C   and is visible from NC iff NJ-NC is not intersected by
      //C   a constraint arc.  Loop on constraints I in reverse
      //C   order --
      //C
      xj = x(nj);
      yj = y(nj);
      ifrst = nn + 1;
      FEM_DOSTEP(i, ncc, 1, -1) {
        ilast = ifrst - 1;
        ifrst = lcc(i);
        nf1 = ilast;
        ncf = nf1 == nc;
        njf = nf1 == nj;
        skip = ncf || njf;
        //C
        //C Loop on boundary constraint arcs NF1-NF2 which contain
        //C   neither NC nor NJ.  NCF and NJF are TRUE iff NC (or NJ)
        //C   has been encountered in the constraint, and SKIP =
        //C   .TRUE. iff NF1 = NC or NF1 = NJ.
        //C
        FEM_DO_SAFE(nf2, ifrst, ilast) {
          if (nf2 == nc) {
            ncf = true;
          }
          if (nf2 == nj) {
            njf = true;
          }
          sksav = skip;
          skip = nf2 == nc || nf2 == nj;
          //C
          //C   The last constraint arc in the constraint need not be
          //C     tested if none of the arcs have been skipped.
          //C
          if (sksav || skip || (nf2 == ilast && !ncf && !njf)) {
            goto statement_9;
          }
          if (intsec(x(nf1), y(nf1), x(nf2), y(nf2), xc, yc, xj, yj)) {
            goto statement_12;
          }
          statement_9:
          nf1 = nf2;
        }
        if (!ncf || !njf) {
          goto statement_11;
        }
        //C
        //C NC and NJ are constraint nodes in the same constraint.
        //C   NC-NJ is intersected by an interior constraint arc iff
        //C   1)  NC LEFT NF2->NF1 and (NJ LEFT NF1->NC and NJ LEFT
        //C         NC->NF2) or
        //C   2)  NC .NOT. LEFT NF2->NF1 and (NJ LEFT NF1->NC or
        //C         NJ LEFT NC->NF2),
        //C   where NF1, NC, NF2 are consecutive constraint nodes.
        //C
        if (nc != ifrst) {
          nf1 = nc - 1;
        }
        else {
          nf1 = ilast;
        }
        if (nc != ilast) {
          nf2 = nc + 1;
        }
        else {
          nf2 = ifrst;
        }
        lft1 = (xc - x(nf1)) * (yj - y(nf1)) >= (xj - x(nf1)) * (yc - y(nf1));
        lft2 = (x(nf2) - xc) * (yj - yc) >= (xj - xc) * (y(nf2) - yc);
        lft12 = (x(nf1) - x(nf2)) * (yc - y(nf2)) >= (xc - x(nf2)) * (
          y(nf1) - y(nf2));
        if ((lft1 && lft2) || (!lft12 && (lft1 || lft2))) {
          goto statement_12;
        }
        statement_11:;
      }
      //C
      //C NJ is visible from NC.  Exit the loop with DC = Euclidean
      //C   distance if J = 1.
      //C
      if (j == 1) {
        goto statement_14;
      }
      dc = fem::min(dc, ds(j) + fem::sqrt((xc - xj) * (xc - xj) + (
        yc - yj) * (yc - yj)));
      goto statement_13;
      //C
      //C NJ is not visible from NC or is adjacent to NC.  Initial-
      //C   ize DC with D(N1,NK) + D(NK,NC) if J = 1.
      //C
      statement_12:
      if (j == 1) {
        dc = ds(k) + fem::sqrt((xc - xk) * (xc - xk) + (yc - yk) * (yc - yk));
      }
      statement_13:;
    }
    //C
    //C Compare DC with DL.
    //C
    if (isw && dc >= dl) {
      goto statement_15;
    }
    //C
    //C The first (or a closer) candidate for NL has been
    //C   encountered.
    //C
    statement_14:
    nl = nc;
    dl = dc;
    isw = true;
    statement_15:
    if (lpk != lpkl) {
      goto statement_7;
    }
  }
  //C
  //C Unmark the elements of NPTS and store NL and DL.
  //C
  FEM_DO_SAFE(k, 1, lm1) {
    nk = npts(k);
    lend(nk) = -lend(nk);
  }
  npts(l) = nl;
  ds(l) = dl;
  ier = 0;
}

int
nearnd(
  common& cmn,
  double const& xp,
  double const& yp,
  int const& ist,
  int const& n,
  arr_cref<double> x,
  arr_cref<double> y,
  arr_cref<int> list,
  arr_cref<int> lptr,
  arr_cref<int> lend,
  double& dsq)
{
  int return_value = fem::int0;
  x(dimension(n));
  y(dimension(n));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  int nst = fem::int0;
  int i1 = fem::int0;
  int i2 = fem::int0;
  int i3 = fem::int0;
  const int lmax = 25;
  arr_1d<lmax, int> listp(fem::fill0);
  arr_1d<lmax, int> lptrp(fem::fill0);
  int l = fem::int0;
  int n1 = fem::int0;
  int lp1 = fem::int0;
  int lpl = fem::int0;
  int lp2 = fem::int0;
  int n2 = fem::int0;
  int lp = fem::int0;
  int n3 = fem::int0;
  double dx11 = fem::double0;
  double dx12 = fem::double0;
  double dx22 = fem::double0;
  double dx21 = fem::double0;
  double dy11 = fem::double0;
  double dy12 = fem::double0;
  double dy22 = fem::double0;
  double dy21 = fem::double0;
  double cos1 = fem::double0;
  double cos2 = fem::double0;
  double sin1 = fem::double0;
  double sin2 = fem::double0;
  int nr = fem::int0;
  double dsr = fem::double0;
  double ds1 = fem::double0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   06/27/98
  //C
  //C   Given a point P in the plane and a Delaunay triangula-
  //C tion created by Subroutine TRMESH or TRMSHR, this function
  //C returns the index of the nearest triangulation node to P.
  //C
  //C   The algorithm consists of implicitly adding P to the
  //C triangulation, finding the nearest neighbor to P, and
  //C implicitly deleting P from the triangulation.  Thus, it
  //C is based on the fact that, if P is a node in a Delaunay
  //C triangulation, the nearest node to P is a neighbor of P.
  //C
  //C On input:
  //C
  //C       XP,YP = Cartesian coordinates of the point P to be
  //C               located relative to the triangulation.
  //C
  //C       IST = Index of a node at which TRFIND begins the
  //C             search.  Search time depends on the proximity
  //C             of this node to P.
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 3.
  //C
  //C       X,Y = Arrays of length N containing the Cartesian
  //C             coordinates of the nodes.
  //C
  //C       LIST,LPTR,LEND = Data structure defining the trian-
  //C                        gulation.  Refer to TRMESH.
  //C
  //C Input parameters are not altered by this function.
  //C
  //C On output:
  //C
  //C       NEARND = Nodal index of the nearest node to P, or 0
  //C                if N < 3 or the triangulation data struc-
  //C                ture is invalid.
  //C
  //C       DSQ = Squared distance between P and NEARND unless
  //C             NEARND = 0.
  //C
  //C       Note that the number of candidates for NEARND
  //C       (neighbors of P) is limited to LMAX defined in
  //C       the PARAMETER statement below.
  //C
  //C Modules required by NEARND:  JRAND, LEFT, LSTPTR, TRFIND
  //C
  //C Intrinsic function called by NEARND:  ABS
  //C
  //C***********************************************************
  //C
  //C Store local parameters and test for N invalid.
  //C
  if (n < 3) {
    goto statement_7;
  }
  nst = ist;
  if (nst < 1 || nst > n) {
    nst = 1;
  }
  //C
  //C Find a triangle (I1,I2,I3) containing P, or the rightmost
  //C   (I1) and leftmost (I2) visible boundary nodes as viewed
  //C   from P.
  //C
  trfind(cmn, nst, xp, yp, n, x, y, list, lptr, lend, i1, i2, i3);
  //C
  //C Test for collinear nodes.
  //C
  if (i1 == 0) {
    goto statement_7;
  }
  //C
  //C Store the linked list of 'neighbors' of P in LISTP and
  //C   LPTRP.  I1 is the first neighbor, and 0 is stored as
  //C   the last neighbor if P is not contained in a triangle.
  //C   L is the length of LISTP and LPTRP, and is limited to
  //C   LMAX.
  //C
  if (i3 != 0) {
    listp(1) = i1;
    lptrp(1) = 2;
    listp(2) = i2;
    lptrp(2) = 3;
    listp(3) = i3;
    lptrp(3) = 1;
    l = 3;
  }
  else {
    n1 = i1;
    l = 1;
    lp1 = 2;
    listp(l) = n1;
    lptrp(l) = lp1;
    //C
    //C   Loop on the ordered sequence of visible boundary nodes
    //C     N1 from I1 to I2.
    //C
    statement_1:
    lpl = lend(n1);
    n1 = -list(lpl);
    l = lp1;
    lp1 = l + 1;
    listp(l) = n1;
    lptrp(l) = lp1;
    if (n1 != i2 && lp1 < lmax) {
      goto statement_1;
    }
    l = lp1;
    listp(l) = 0;
    lptrp(l) = 1;
  }
  //C
  //C Initialize variables for a loop on arcs N1-N2 opposite P
  //C   in which new 'neighbors' are 'swapped' in.  N1 follows
  //C   N2 as a neighbor of P, and LP1 and LP2 are the LISTP
  //C   indexes of N1 and N2.
  //C
  lp2 = 1;
  n2 = i1;
  lp1 = lptrp(1);
  n1 = listp(lp1);
  //C
  //C Begin loop:  find the node N3 opposite N1->N2.
  //C
  statement_2:
  lp = lstptr(lend(n1), n2, list, lptr);
  if (list(lp) < 0) {
    goto statement_4;
  }
  lp = lptr(lp);
  n3 = fem::abs(list(lp));
  //C
  //C Swap test:  Exit the loop if L = LMAX.
  //C
  if (l == lmax) {
    goto statement_5;
  }
  dx11 = x(n1) - x(n3);
  dx12 = x(n2) - x(n3);
  dx22 = x(n2) - xp;
  dx21 = x(n1) - xp;
  //C
  dy11 = y(n1) - y(n3);
  dy12 = y(n2) - y(n3);
  dy22 = y(n2) - yp;
  dy21 = y(n1) - yp;
  //C
  cos1 = dx11 * dx12 + dy11 * dy12;
  cos2 = dx22 * dx21 + dy22 * dy21;
  if (cos1 >= 0.f && cos2 >= 0.f) {
    goto statement_4;
  }
  if (cos1 < 0.f && cos2 < 0.f) {
    goto statement_3;
  }
  //C
  sin1 = dx11 * dy12 - dx12 * dy11;
  sin2 = dx22 * dy21 - dx21 * dy22;
  if (sin1 * cos2 + cos1 * sin2 >= 0.f) {
    goto statement_4;
  }
  //C
  //C Swap:  Insert N3 following N2 in the adjacency list for P.
  //C        The two new arcs opposite P must be tested.
  //C
  statement_3:
  l++;
  lptrp(lp2) = l;
  listp(l) = n3;
  lptrp(l) = lp1;
  lp1 = l;
  n1 = n3;
  goto statement_2;
  //C
  //C No swap:  Advance to the next arc and test for termination
  //C           on N1 = I1 (LP1 = 1) or N1 followed by 0.
  //C
  statement_4:
  if (lp1 == 1) {
    goto statement_5;
  }
  lp2 = lp1;
  n2 = n1;
  lp1 = lptrp(lp1);
  n1 = listp(lp1);
  if (n1 == 0) {
    goto statement_5;
  }
  goto statement_2;
  //C
  //C Set NR and DSR to the index of the nearest node to P and
  //C   its squared distance from P, respectively.
  //C
  statement_5:
  nr = i1;
  dsr = fem::pow2((x(nr) - xp)) + fem::pow2((y(nr) - yp));
  FEM_DO_SAFE(lp, 2, l) {
    n1 = listp(lp);
    if (n1 == 0) {
      goto statement_6;
    }
    ds1 = fem::pow2((x(n1) - xp)) + fem::pow2((y(n1) - yp));
    if (ds1 < dsr) {
      nr = n1;
      dsr = ds1;
    }
    statement_6:;
  }
  dsq = dsr;
  return_value = nr;
  return return_value;
  //C
  //C Invalid input.
  //C
  statement_7:
  return_value = 0;
  return return_value;
}

struct trlprt_save
{
  int nlmax;
  int nmax;

  trlprt_save() :
    nlmax(fem::int0),
    nmax(fem::int0)
  {}
};

void
trlprt(
  common& cmn,
  int const& /* ncc */,
  arr_cref<int> /* lct */,
  int const& n,
  arr_cref<double> /* x */,
  arr_cref<double> /* y */,
  int const& nrow,
  int const& nt,
  arr_cref<int, 2> /* ltri */,
  int const& lout,
  bool const& prntx)
{
  FEM_CMN_SVE(trlprt);
  int& nlmax = sve.nlmax;
  int& nmax = sve.nmax;
  if (is_called_first_time) {
    nmax = 9999;
    nlmax = 60;
  }
  int lun = fem::int0;
  int nl = fem::int0;
  int i = fem::int0;
  int k = fem::int0;
  int nb = fem::int0;
  int na = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRLPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   07/02/98
  //C
  //C   Given a triangulation of a set of points in the plane,
  //C this subroutine prints the triangle list created by
  //C Subroutine TRLIST and, optionally, the nodal coordinates
  //C on logical unit LOUT.  The numbers of boundary nodes,
  //C triangles, and arcs, and the constraint region triangle
  //C indexes, if any, are also printed.
  //C
  //C   All parameters other than LOUT and PRNTX should be
  //C unaltered from their values on output from TRLIST.
  //C
  //C On input:
  //C
  //C       NCC = Number of constraints.
  //C
  //C       LCT = List of constraint triangle starting indexes
  //C             (or dummy array of length 1 if NCC = 0).
  //C
  //C       N = Number of nodes in the triangulation.
  //C           3 .LE. N .LE. 9999.
  //C
  //C       X,Y = Arrays of length N containing the coordinates
  //C             of the nodes in the triangulation -- not used
  //C             unless PRNTX = TRUE.
  //C
  //C       NROW = Number of rows (entries per triangle) re-
  //C              served for the triangle list LTRI.  The value
  //C              must be 6 if only the vertex indexes and
  //C              neighboring triangle indexes are stored, or 9
  //C              if arc indexes are also stored.
  //C
  //C       NT = Number of triangles in the triangulation.
  //C            1 .LE. NT .LE. 9999.
  //C
  //C       LTRI = NROW by NT array whose J-th column contains
  //C              the vertex nodal indexes (first three rows),
  //C              neighboring triangle indexes (second three
  //C              rows), and, if NROW = 9, arc indexes (last
  //C              three rows) associated with triangle J for
  //C              J = 1,...,NT.
  //C
  //C       LOUT = Logical unit number for output.  0 .LE. LOUT
  //C              .LE. 99.  Output is printed on unit 6 if LOUT
  //C              is outside its valid range on input.
  //C
  //C       PRNTX = Logical variable with value TRUE if and only
  //C               if X and Y are to be printed (to 6 decimal
  //C               places).
  //C
  //C None of the parameters are altered by this routine.
  //C
  //C Modules required by TRLPRT:  None
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C   I = DO-loop, nodal index, and row index for LTRI
  //C   K = DO-loop and triangle index
  //C   LUN = Logical unit number for output
  //C   NA = Number of triangulation arcs
  //C   NB = Number of boundary nodes
  //C   NL = Number of lines printed on the current page
  //C   NLMAX = Maximum number of print lines per page
  //C   NMAX = Maximum value of N and NT (4-digit format)
  //C
  lun = lout;
  if (lun < 0 || lun > 99) {
    lun = 6;
  }
  //C
  //C Print a heading and test for invalid input.
  //C
  //C       WRITE (LUN,100)
  nl = 1;
  if (n < 3 || n > nmax || (nrow != 6 && nrow != 9) || nt < 1 || nt > nmax) {
    //C
    //C Print an error message and bypass the loops.
    //C
    //C       WRITE (LUN,110) N, NROW, NT
    goto statement_3;
  }
  if (prntx) {
    //C
    //C Print X and Y.
    //C
    //C       WRITE (LUN,101)
    nl = 6;
    FEM_DO_SAFE(i, 1, n) {
      if (nl >= nlmax) {
        //C       WRITE (LUN,106)
        nl = 0;
      }
      //C       WRITE (LUN,102) I, X(I), Y(I)
      nl++;
    }
  }
  //C
  //C Print the triangulation LTRI.
  //C
  if (nl > nlmax / 2) {
    //C       WRITE (LUN,106)
    nl = 0;
  }
  if (nrow == 6) {
    //C       WRITE (LUN,103)
  }
  else {
    //C       WRITE (LUN,104)
  }
  nl += 5;
  FEM_DO_SAFE(k, 1, nt) {
    if (nl >= nlmax) {
      //C       WRITE (LUN,106)
      nl = 0;
    }
    //C       WRITE (LUN,105) K, (LTRI(I,K), I = 1,NROW)
    nl++;
  }
  //C
  //C Print NB, NA, and NT (boundary nodes, arcs, and
  //C   triangles).
  //C
  nb = 2 * n - nt - 2;
  na = nt + n - 1;
  //C      IF (NL .GT. NLMAX-6) WRITE (LUN,106)
  //C       WRITE (LUN,107) NB, NA, NT
  //C
  //C Print NCC and LCT.
  //C
  statement_3:;
  //C    3 WRITE (LUN,108) NCC
  //C      IF (NCC .GT. 0) WRITE (LUN,109) (LCT(I), I = 1,NCC)
  //C
  //C Print formats:
  //C
  (void)nb;
  (void)na;
}

void
trmshr(
  common& cmn,
  int const& n,
  int const& nx,
  arr_cref<double> x,
  arr_cref<double> y,
  int& nit,
  arr_ref<int> list,
  arr_ref<int> lptr,
  arr_ref<int> lend,
  int& lnew,
  int& ier)
{
  x(dimension(n));
  y(dimension(n));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  double& swtol = cmn.swtol;
  //
  int ni = fem::int0;
  int nj = fem::int0;
  int nn = fem::int0;
  int maxit = fem::int0;
  double eps = fem::double0;
  bool tst = fem::bool0;
  int m1 = fem::int0;
  int m4 = fem::int0;
  int lp = fem::int0;
  int kp1 = fem::int0;
  int j = fem::int0;
  int i = fem::int0;
  int m2 = fem::int0;
  int m3 = fem::int0;
  int k = fem::int0;
  int lpf = fem::int0;
  int lpk = fem::int0;
  int n0 = fem::int0;
  int n1 = fem::int0;
  int n2 = fem::int0;
  int lpl = fem::int0;
  int n3 = fem::int0;
  int iter = fem::int0;
  int nm1 = fem::int0;
  int n4 = fem::int0;
  int nnb = fem::int0;
  int lpp = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   06/27/98
  //C
  //C   This subroutine creates a Delaunay triangulation of a
  //C set of N nodes in the plane, where the nodes are the vert-
  //C ices of an NX by NY skewed rectangular grid with the
  //C natural ordering.  Thus, N = NX*NY, and the nodes are
  //C ordered from left to right beginning at the top row so
  //C that adjacent nodes have indexes which differ by 1 in the
  //C x-direction and by NX in the y-direction.  A skewed rec-
  //C tangular grid is defined as one in which each grid cell is
  //C a strictly convex quadrilateral (and is thus the convex
  //C hull of its four vertices).  Equivalently, any transfor-
  //C mation from a rectangle to a grid cell which is bilinear
  //C in both components has an invertible Jacobian.
  //C
  //C   If the nodes are not distributed and ordered as defined
  //C above, Subroutine TRMESH must be called in place of this
  //C routine.  Refer to Subroutine ADDCST for the treatment of
  //C constraints.
  //C
  //C   The first phase of the algorithm consists of construc-
  //C ting a triangulation by choosing a diagonal arc in each
  //C grid cell.  If NIT = 0, all diagonals connect lower left
  //C to upper right corners and no error checking or additional
  //C computation is performed.  Otherwise, each diagonal arc is
  //C chosen to be locally optimal, and boundary arcs are added
  //C where necessary in order to cover the convex hull of the
  //C nodes.  (This is the first iteration.)  If NIT > 1 and no
  //C error was detected, the triangulation is then optimized by
  //C a sequence of up to NIT-1 iterations in which interior
  //C arcs of the triangulation are tested and swapped if appro-
  //C priate.  The algorithm terminates when an iteration
  //C results in no swaps and/or when the allowable number of
  //C iterations has been performed.  NIT = 0 is sufficient to
  //C produce a Delaunay triangulation if the original grid is
  //C actually rectangular, and NIT = 1 is sufficient if it is
  //C close to rectangular.  Note, however, that the ordering
  //C and distribution of nodes is not checked for validity in
  //C the case NIT = 0, and the triangulation will not be valid
  //C unless the rectangular grid covers the convex hull of the
  //C nodes.
  //C
  //C On input:
  //C
  //C       N = Number of nodes in the grid.  N = NX*NY for some
  //C           NY .GE. 2.
  //C
  //C       NX = Number of grid points in the x-direction.  NX
  //C            .GE. 2.
  //C
  //C       X,Y = Arrays of length N containing coordinates of
  //C             the nodes with the ordering and distribution
  //C             defined in the header comments above.
  //C             (X(K),Y(K)) is referred to as node K.
  //C
  //C The above parameters are not altered by this routine.
  //C
  //C       NIT = Nonnegative integer specifying the maximum
  //C             number of iterations to be employed.  Refer
  //C             to the header comments above.
  //C
  //C       LIST,LPTR = Arrays of length at least 6N-12.
  //C
  //C       LEND = Array of length at least N.
  //C
  //C On output:
  //C
  //C       NIT = Number of iterations employed.
  //C
  //C       LIST,LPTR,LEND,LNEW = Data structure defining the
  //C                             triangulation.  Refer to Sub-
  //C                             routine TRMESH.
  //C
  //C       IER = Error indicator:
  //C             IER = 0 if no errors were encountered.
  //C             IER = K if the grid element with upper left
  //C                     corner at node K is not a strictly
  //C                     convex quadrilateral.  The algorithm
  //C                     is terminated when the first such
  //C                     occurrence is detected.  Note that
  //C                     this test is not performed if NIT = 0
  //C                     on input.
  //C             IER = -1 if N, NX, or NIT is outside its valid
  //C                      range on input.
  //C             IER = -2 if NIT > 1 on input, and the optimi-
  //C                      zation loop failed to converge within
  //C                      the allowable number of iterations.
  //C                      The triangulation is valid but not
  //C                      optimal in this case.
  //C
  //C Modules required by TRMSHR:  INSERT, LEFT, LSTPTR, NBCNT,
  //C                                STORE, SWAP, SWPTST
  //C
  //C Intrinsic function called by TRMSHR:  ABS
  //C
  //C***********************************************************
  //C
  //C Store local variables and test for errors in input
  //C   parameters.
  //C
  ni = nx;
  nj = n / ni;
  nn = ni * nj;
  maxit = nit;
  nit = 0;
  if (n != nn || nj < 2 || ni < 2 || maxit < 0) {
    ier = -1;
    return;
  }
  ier = 0;
  //C
  //C Compute a tolerance for function SWPTST:  SWTOL = 10*
  //C   (machine precision)
  //C
  eps = 1.f;
  statement_1:
  eps = eps / 2.f;
  swtol = store(cmn, eps + 1.f);
  if (swtol > 1.f) {
    goto statement_1;
  }
  swtol = eps * 20.f;
  //C
  //C Loop on grid points (I,J) corresponding to nodes K =
  //C   (J-1)*NI + I.  TST = TRUE iff diagonals are to be
  //C   chosen by the swap test.  M1, M2, M3, and M4 are the
  //C   slopes (-1, 0, or 1) of the diagonals in quadrants 1
  //C   to 4 (counterclockwise beginning with the upper right)
  //C   for a coordinate system with origin at node K.
  //C
  tst = maxit > 0;
  m1 = 0;
  m4 = 0;
  lp = 0;
  kp1 = 1;
  FEM_DO_SAFE(j, 1, nj) {
    FEM_DO_SAFE(i, 1, ni) {
      m2 = m1;
      m3 = m4;
      k = kp1;
      kp1 = k + 1;
      lpf = lp + 1;
      if (j == nj && i != ni) {
        goto statement_2;
      }
      if (i != 1) {
        if (j != 1) {
          //C
          //C   K is not in the top row, leftmost column, or bottom row
          //C     (unless K is the lower right corner).  Take the first
          //C     neighbor to be the node above K.
          //C
          lp++;
          list(lp) = k - ni;
          lptr(lp) = lp + 1;
          if (m2 <= 0) {
            lp++;
            list(lp) = k - 1 - ni;
            lptr(lp) = lp + 1;
          }
        }
        //C
        //C   K is not in the leftmost column.  The next (or first)
        //C     neighbor is to the left of K.
        //C
        lp++;
        list(lp) = k - 1;
        lptr(lp) = lp + 1;
        if (j == nj) {
          goto statement_3;
        }
        if (m3 >= 0) {
          lp++;
          list(lp) = k - 1 + ni;
          lptr(lp) = lp + 1;
        }
      }
      //C
      //C   K is not in the bottom row.  The next (or first)
      //C     neighbor is below K.
      //C
      lp++;
      list(lp) = k + ni;
      lptr(lp) = lp + 1;
      //C
      //C   Test for a negative diagonal in quadrant 4 unless K is
      //C     in the rightmost column.  The quadrilateral associated
      //C     with the quadrant is tested for strict convexity un-
      //C     less NIT = 0 on input.
      //C
      if (i == ni) {
        goto statement_3;
      }
      m4 = 1;
      if (!tst) {
        goto statement_2;
      }
      if (left(x(kp1), y(kp1), x(k + ni), y(k + ni), x(k), y(k)) || left(x(k),
          y(k), x(kp1 + ni), y(kp1 + ni), x(k + ni), y(k + ni)) ||
          left(x(k + ni), y(k + ni), x(kp1), y(kp1), x(kp1 + ni), y(
          kp1 + ni)) || left(x(kp1 + ni), y(kp1 + ni), x(k), y(k), x(kp1),
          y(kp1))) {
        goto statement_12;
      }
      if (swptst(cmn, kp1, k + ni, k, kp1 + ni, x, y)) {
        goto statement_2;
      }
      m4 = -1;
      lp++;
      list(lp) = kp1 + ni;
      lptr(lp) = lp + 1;
      //C
      //C   The next (or first) neighbor is to the right of K.
      //C
      statement_2:
      lp++;
      list(lp) = kp1;
      lptr(lp) = lp + 1;
      //C
      //C   Test for a positive diagonal in quadrant 1 (the neighbor
      //C     of K-NI which follows K is not K+1) unless K is in the
      //C     top row.
      //C
      if (j == 1) {
        goto statement_3;
      }
      if (tst) {
        m1 = -1;
        lpk = lstptr(lend(k - ni), k, list, lptr);
        lpk = lptr(lpk);
        if (list(lpk) != kp1) {
          m1 = 1;
          lp++;
          list(lp) = kp1 - ni;
          lptr(lp) = lp + 1;
        }
      }
      //C
      //C   If K is in the leftmost column (and not the top row) or
      //C     in the bottom row (and not the rightmost column), then
      //C     the next neighbor is the node above K.
      //C
      if (i != 1 && j != nj) {
        goto statement_4;
      }
      lp++;
      list(lp) = k - ni;
      lptr(lp) = lp + 1;
      if (i == 1) {
        goto statement_3;
      }
      //C
      //C   K is on the bottom row (and not the leftmost or right-
      //C     most column).
      //C
      if (m2 <= 0) {
        lp++;
        list(lp) = k - 1 - ni;
        lptr(lp) = lp + 1;
      }
      lp++;
      list(lp) = k - 1;
      lptr(lp) = lp + 1;
      //C
      //C   K is a boundary node.
      //C
      statement_3:
      list(lp) = -list(lp);
      //C
      //C   Bottom of loop.  Store LEND and correct LPTR(LP).
      //C     LPF and LP point to the first and last neighbors
      //C     of K.
      //C
      statement_4:
      lend(k) = lp;
      lptr(lp) = lpf;
    }
  }
  //C
  //C Store LNEW, and terminate the algorithm if NIT = 0 on
  //C   input.
  //C
  lnew = lp + 1;
  if (maxit == 0) {
    return;
  }
  //C
  //C Add boundary arcs where necessary in order to cover the
  //C   convex hull of the nodes.  N1, N2, and N3 are consecu-
  //C   tive boundary nodes in counterclockwise order, and N0
  //C   is the starting point for each loop around the boundary.
  //C
  n0 = 1;
  n1 = n0;
  n2 = ni + 1;
  //C
  //C   TST is set to TRUE if an arc is added.  The boundary
  //C     loop is repeated until a traversal results in no
  //C     added arcs.
  //C
  statement_7:
  tst = false;
  //C
  //C   Top of boundary loop.  Set N3 to the first neighbor of
  //C     N2, and test for N3 LEFT N1 -> N2.
  //C
  statement_8:
  lpl = lend(n2);
  lp = lptr(lpl);
  n3 = list(lp);
  if (left(x(n1), y(n1), x(n2), y(n2), x(n3), y(n3))) {
    n1 = n2;
  }
  if (n1 != n2) {
    //C
    //C   Add the boundary arc N1-N3.  If N0 = N2, the starting
    //C     point is changed to N3, since N2 will be removed from
    //C     the boundary.  N3 is inserted as the first neighbor of
    //C     N1, N2 is changed to an interior node, and N1 is
    //C     inserted as the last neighbor of N3.
    //C
    tst = true;
    if (n2 == n0) {
      n0 = n3;
    }
    lp = lend(n1);
    insert(n3, lp, list, lptr, lnew);
    list(lpl) = -list(lpl);
    lp = lend(n3);
    list(lp) = n2;
    insert(-n1, lp, list, lptr, lnew);
    lend(n3) = lnew - 1;
  }
  //C
  //C   Bottom of loops.  Test for termination.
  //C
  n2 = n3;
  if (n1 != n0) {
    goto statement_8;
  }
  if (tst) {
    goto statement_7;
  }
  //C
  //C Terminate the algorithm if NIT = 1 on input.
  //C
  nit = 1;
  if (maxit == 1) {
    return;
  }
  //C
  //C Optimize the triangulation by applying the swap test and
  //C   appropriate swaps to the interior arcs.  The loop is
  //C   repeated until no swaps are performed or MAXIT itera-
  //C   tions have been applied.  ITER is the current iteration,
  //C   and TST is set to TRUE if a swap occurs.
  //C
  iter = 1;
  nm1 = nn - 1;
  statement_9:
  iter++;
  tst = false;
  //C
  //C   Loop on interior arcs N1-N2, where N2 > N1 and
  //C     (N1,N2,N3) and (N2,N1,N4) are adjacent triangles.
  //C
  //C   Top of loop on nodes N1.
  //C
  FEM_DO_SAFE(n1, 1, nm1) {
    lpl = lend(n1);
    n4 = list(lpl);
    lpf = lptr(lpl);
    n2 = list(lpf);
    lp = lptr(lpf);
    n3 = list(lp);
    nnb = nbcnt(lpl, lptr);
    //C
    //C   Top of loop on neighbors N2 of N1.  NNB is the number of
    //C                                       neighbors of N1.
    //C
    FEM_DO_SAFE(i, 1, nnb) {
      //C
      //C   Bypass the swap test if N1 is a boundary node and N2 is
      //C     the first neighbor (N4 < 0), N2 < N1, or N1-N2 is a
      //C     diagonal arc (already locally optimal) when ITER = 2.
      //C
      if (n4 > 0 && n2 > n1 && (iter != 2 || fem::abs(n1 + ni - n2) != 1)) {
        if (swptst(cmn, n3, n4, n1, n2, x, y)) {
          //C
          //C   Swap diagonal N1-N2 for N3-N4, set TST to TRUE, and set
          //C     N2 to N4 (the neighbor preceding N3).
          //C
          swap(n3, n4, n1, n2, list, lptr, lend, lpp);
          if (lpp != 0) {
            tst = true;
            n2 = n4;
          }
        }
      }
      //C
      //C   Bottom of neighbor loop.
      //C
      if (list(lpl) ==  - n3) {
        goto statement_11;
      }
      n4 = n2;
      n2 = n3;
      lp = lstptr(lpl, n2, list, lptr);
      lp = lptr(lp);
      n3 = fem::abs(list(lp));
    }
    statement_11:;
  }
  //C
  //C   Test for termination.
  //C
  if (tst && iter < maxit) {
    goto statement_9;
  }
  nit = iter;
  if (tst) {
    ier = -2;
  }
  return;
  //C
  //C Invalid grid cell encountered.
  //C
  statement_12:
  ier = k;
}

struct trplot_save
{
  bool annot;
  double dashl;
  double fsizn;
  double fsizt;

  trplot_save() :
    annot(fem::bool0),
    dashl(fem::double0),
    fsizn(fem::double0),
    fsizt(fem::double0)
  {}
};

void
trplot(
  common& cmn,
  int const& lun,
  double const& pltsiz,
  double const& wx1,
  double const& wx2,
  double const& wy1,
  double const& wy2,
  int const& ncc,
  arr_cref<int> lcc,
  int const& n,
  arr_cref<double> x,
  arr_cref<double> y,
  arr_cref<int> list,
  arr_cref<int> lptr,
  arr_cref<int> lend,
  str_cref /* title */,
  bool const& numbr,
  int& ier)
{
  FEM_CMN_SVE(trplot);
  lcc(dimension(star));
  x(dimension(n));
  y(dimension(n));
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  bool& annot = sve.annot;
  double& dashl = sve.dashl;
  double& fsizn = sve.fsizn;
  double& fsizt = sve.fsizt;
  if (is_called_first_time) {
    annot = true;
    dashl = 4.0f;
    fsizn = 10.0f;
    fsizt = 16.0f;
  }
  int nls = fem::int0;
  double dx = fem::double0;
  double dy = fem::double0;
  double r = fem::double0;
  double t = fem::double0;
  int ipx1 = fem::int0;
  int ipx2 = fem::int0;
  int ipy1 = fem::int0;
  int ipy2 = fem::int0;
  int iw = fem::int0;
  int ih = fem::int0;
  double sfx = fem::double0;
  double sfy = fem::double0;
  double tx = fem::double0;
  double ty = fem::double0;
  int n0 = fem::int0;
  double x0 = fem::double0;
  double y0 = fem::double0;
  int lpl = fem::int0;
  int lp = fem::int0;
  int n1 = fem::int0;
  bool pass1 = fem::bool0;
  int ifrst = fem::int0;
  int i = fem::int0;
  int ilast = fem::int0;
  int n0bak = fem::int0;
  int n0for = fem::int0;
  bool cnstr = fem::bool0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   07/15/98
  //C
  //C   This subroutine creates a level-2 Encapsulated Post-
  //C script (EPS) file containing a triangulation plot.
  //C
  //C On input:
  //C
  //C       LUN = Logical unit number in the range 0 to 99.
  //C             The unit should be opened with an appropriate
  //C             file name before the call to this routine.
  //C
  //C       PLTSIZ = Plot size in inches.  The window is mapped,
  //C                with aspect ratio preserved, to a rectangu-
  //C                lar viewport with maximum side-length equal
  //C                to .88*PLTSIZ (leaving room for labels out-
  //C                side the viewport).  The viewport is
  //C                centered on the 8.5 by 11 inch page, and
  //C                its boundary is drawn.  1.0 .LE. PLTSIZ
  //C                .LE. 8.5.
  //C
  //C       WX1,WX2,WY1,WY2 = Parameters defining a rectangular
  //C                         window against which the triangu-
  //C                         lation is clipped.  (Only the
  //C                         portion of the triangulation that
  //C                         lies in the window is drawn.)
  //C                         (WX1,WY1) and (WX2,WY2) are the
  //C                         lower left and upper right cor-
  //C                         ners, respectively.  WX1 < WX2 and
  //C                         WY1 < WY2.
  //C
  //C       NCC = Number of constraint curves.  Refer to Subrou-
  //C             tine ADDCST.  NCC .GE. 0.
  //C
  //C       LCC = Array of length NCC (or dummy parameter if
  //C             NCC = 0) containing the index of the first
  //C             node of constraint I in LCC(I).  For I = 1 to
  //C             NCC, LCC(I+1)-LCC(I) .GE. 3, where LCC(NCC+1)
  //C             = N+1.
  //C
  //C       N = Number of nodes in the triangulation.  N .GE. 3.
  //C
  //C       X,Y = Arrays of length N containing the coordinates
  //C             of the nodes with non-constraint nodes in the
  //C             first LCC(1)-1 locations.
  //C
  //C       LIST,LPTR,LEND = Data structure defining the trian-
  //C                        gulation.  Refer to Subroutine
  //C                        TRMESH.
  //C
  //C       TITLE = Type CHARACTER variable or constant contain-
  //C               ing a string to be centered above the plot.
  //C               The string must be enclosed in parentheses;
  //C               i.e., the first and last characters must be
  //C               '(' and ')', respectively, but these are not
  //C               displayed.  TITLE may have at most 80 char-
  //C               acters including the parentheses.
  //C
  //C       NUMBR = Option indicator:  If NUMBR = TRUE, the
  //C               nodal indexes are plotted next to the nodes.
  //C
  //C Input parameters are not altered by this routine.
  //C
  //C On output:
  //C
  //C       IER = Error indicator:
  //C             IER = 0 if no errors were encountered.
  //C             IER = 1 if LUN, PLTSIZ, NCC, or N is outside
  //C                     its valid range.  LCC is not tested
  //C                     for validity.
  //C             IER = 2 if WX1 >= WX2 or WY1 >= WY2.
  //C             IER = 3 if an error was encountered in writing
  //C                     to unit LUN.
  //C
  //C   Various plotting options can be controlled by altering
  //C the data statement below.
  //C
  //C Modules required by TRPLOT:  None
  //C
  //C Intrinsic functions called by TRPLOT:  ABS, CHAR, NINT,
  //C                                          DBLE
  //C
  //C***********************************************************
  //C
  //C Local parameters:
  //C
  //C ANNOT =     Logical variable with value TRUE iff the plot
  //C               is to be annotated with the values of WX1,
  //C               WX2, WY1, and WY2
  //C CNSTR       Logical variable used to flag constraint arcs:
  //C               TRUE iff N0-N1 lies in a constraint region
  //C DASHL =     Length (in points, at 72 points per inch) of
  //C               dashes and spaces in a dashed line pattern
  //C               used for drawing constraint arcs
  //C DX =        Window width WX2-WX1
  //C DY =        Window height WY2-WY1
  //C FSIZN =     Font size in points for labeling nodes with
  //C               their indexes if NUMBR = TRUE
  //C FSIZT =     Font size in points for the title (and
  //C               annotation if ANNOT = TRUE)
  //C I =         Constraint index (1 to NCC)
  //C IFRST =     Index of the first node in constraint I
  //C IH =        Height of the viewport in points
  //C ILAST =     Index of the last node in constraint I
  //C IPX1,IPY1 = X and y coordinates (in points) of the lower
  //C               left corner of the bounding box or viewport
  //C IPX2,IPY2 = X and y coordinates (in points) of the upper
  //C               right corner of the bounding box or viewport
  //C IW =        Width of the viewport in points
  //C LP =        LIST index (pointer)
  //C LPL =       Pointer to the last neighbor of N0
  //C N0 =        Nodal index and DO-loop index
  //C N0BAK =     Predecessor of N0 in a constraint curve
  //C               (sequence of adjacent constraint nodes)
  //C N0FOR =     Successor to N0 in a constraint curve
  //C N1 =        Index of a neighbor of N0
  //C NLS =       Index of the last non-constraint node
  //C PASS1 =     Logical variable used to flag the first pass
  //C               through the constraint nodes
  //C R =         Aspect ratio DX/DY
  //C SFX,SFY =   Scale factors for mapping world coordinates
  //C               (window coordinates in [WX1,WX2] X [WY1,WY2])
  //C               to viewport coordinates in [IPX1,IPX2] X
  //C               [IPY1,IPY2]
  //C T =         Temporary variable
  //C TX,TY =     Translation vector for mapping world coordi-
  //C               nates to viewport coordinates
  //C X0,Y0 =     X(N0),Y(N0) or label location
  //C
  //C Test for error 1, and set NLS to the last non-constraint
  //C   node.
  //C
  if (lun < 0 || lun > 99 || pltsiz < 1.0f || pltsiz > 8.5f ||
      ncc < 0 || n < 3) {
    goto statement_11;
  }
  nls = n;
  if (ncc > 0) {
    nls = lcc(1) - 1;
  }
  //C
  //C Compute the aspect ratio of the window.
  //C
  dx = wx2 - wx1;
  dy = wy2 - wy1;
  if (dx <= 0.0f || dy <= 0.0f) {
    goto statement_12;
  }
  r = dx / dy;
  //C
  //C Compute the lower left (IPX1,IPY1) and upper right
  //C   (IPX2,IPY2) corner coordinates of the bounding box.
  //C   The coordinates, specified in default user space units
  //C   (points, at 72 points/inch with origin at the lower
  //C   left corner of the page), are chosen to preserve the
  //C   aspect ratio R, and to center the plot on the 8.5 by 11
  //C   inch page.  The center of the page is (306,396), and
  //C   T = PLTSIZ/2 in points.
  //C
  t = 36.0f * pltsiz;
  if (r >= 1.0f) {
    ipx1 = 306 - fem::nint(t);
    ipx2 = 306 + fem::nint(t);
    ipy1 = 396 - fem::nint(t / r);
    ipy2 = 396 + fem::nint(t / r);
  }
  else {
    ipx1 = 306 - fem::nint(t * r);
    ipx2 = 306 + fem::nint(t * r);
    ipy1 = 396 - fem::nint(t);
    ipy2 = 396 + fem::nint(t);
  }
  //C
  //C Output header comments.
  //C
  //C       WRITE (LUN,100,ERR=13) IPX1, IPY1, IPX2, IPY2
  //C
  //C Set (IPX1,IPY1) and (IPX2,IPY2) to the corner coordinates
  //C   of a viewport obtained by shrinking the bounding box by
  //C   12% in each dimension.
  //C
  iw = fem::nint(0.88f * fem::dble(ipx2 - ipx1));
  ih = fem::nint(0.88f * fem::dble(ipy2 - ipy1));
  ipx1 = 306 - iw / 2;
  ipx2 = 306 + iw / 2;
  ipy1 = 396 - ih / 2;
  ipy2 = 396 + ih / 2;
  //C
  //C Set the line thickness to 2 points, and draw the
  //C   viewport boundary.
  //C
  t = 2.0f;
  //C       WRITE (LUN,110,ERR=13) T
  //C       WRITE (LUN,120,ERR=13) IPX1, IPY1
  //C       WRITE (LUN,130,ERR=13) IPX1, IPY2
  //C       WRITE (LUN,130,ERR=13) IPX2, IPY2
  //C       WRITE (LUN,130,ERR=13) IPX2, IPY1
  //C       WRITE (LUN,140,ERR=13)
  //C       WRITE (LUN,150,ERR=13)
  //C
  //C Set up a mapping from the window to the viewport.
  //C
  sfx = fem::dble(iw) / dx;
  sfy = fem::dble(ih) / dy;
  tx = ipx1 - sfx * wx1;
  ty = ipy1 - sfy * wy1;
  //C       WRITE (LUN,160,ERR=13) TX, TY, SFX, SFY
  //C
  //C The line thickness (believe it or fucking not) must be
  //C   changed to reflect the new scaling which is applied to
  //C   all subsequent output.  Set it to 1.0 point.
  //C
  t = 2.0f / (sfx + sfy);
  //C       WRITE (LUN,110,ERR=13) T
  //C
  //C Save the current graphics state, and set the clip path to
  //C   the boundary of the window.
  //C
  //C       WRITE (LUN,170,ERR=13)
  //C       WRITE (LUN,180,ERR=13) WX1, WY1
  //C       WRITE (LUN,190,ERR=13) WX2, WY1
  //C       WRITE (LUN,190,ERR=13) WX2, WY2
  //C       WRITE (LUN,190,ERR=13) WX1, WY2
  //C       WRITE (LUN,200,ERR=13)
  //C
  //C Draw the edges N0->N1, where N1 > N0, beginning with a
  //C   loop on non-constraint nodes N0.  LPL points to the
  //C   last neighbor of N0.
  //C
  FEM_DO_SAFE(n0, 1, nls) {
    x0 = x(n0);
    y0 = y(n0);
    lpl = lend(n0);
    lp = lpl;
    //C
    //C   Loop on neighbors N1 of N0.
    //C
    statement_2:
    lp = lptr(lp);
    n1 = fem::abs(list(lp));
    if (n1 > n0) {
      //C
      //C   Add the edge to the path.
      //C
      //C       WRITE (LUN,210,ERR=13) X0, Y0, X(N1), Y(N1)
    }
    if (lp != lpl) {
      goto statement_2;
    }
  }
  //C
  //C Loop through the constraint nodes twice.  The non-
  //C   constraint arcs incident on constraint nodes are
  //C   drawn (with solid lines) on the first pass, and the
  //C   constraint arcs (both boundary and interior, if any)
  //C   are drawn (with dashed lines) on the second pass.
  //C
  pass1 = true;
  //C
  //C Loop on constraint nodes N0 with (N0BAK,N0,N0FOR) a sub-
  //C   sequence of constraint I.  The outer loop is on
  //C   constraints I with first and last nodes IFRST and ILAST.
  //C
  statement_4:
  ifrst = n + 1;
  FEM_DOSTEP(i, ncc, 1, -1) {
    ilast = ifrst - 1;
    ifrst = lcc(i);
    n0bak = ilast;
    FEM_DO_SAFE(n0, ifrst, ilast) {
      n0for = n0 + 1;
      if (n0 == ilast) {
        n0for = ifrst;
      }
      lpl = lend(n0);
      x0 = x(n0);
      y0 = y(n0);
      lp = lpl;
      //C
      //C   Loop on neighbors N1 of N0.  CNSTR = TRUE iff N0-N1 is a
      //C     constraint arc.
      //C
      //C   Initialize CNSTR to TRUE iff the first neighbor of N0
      //C     strictly follows N0FOR and precedes or coincides with
      //C     N0BAK (in counterclockwise order).
      //C
      statement_5:
      lp = lptr(lp);
      n1 = fem::abs(list(lp));
      if (n1 != n0for && n1 != n0bak) {
        goto statement_5;
      }
      cnstr = n1 == n0bak;
      lp = lpl;
      //C
      //C   Loop on neighbors N1 of N0.  Update CNSTR and test for
      //C     N1 > N0.
      //C
      statement_6:
      lp = lptr(lp);
      n1 = fem::abs(list(lp));
      if (n1 == n0for) {
        cnstr = true;
      }
      if (n1 > n0) {
        //C
        //C   Draw the edge iff (PASS1=TRUE and CNSTR=FALSE) or
        //C     (PASS1=FALSE and CNSTR=TRUE); i.e., CNSTR and PASS1
        //C     have opposite values.
        //C
        //C             IF (CNSTR .NEQV. PASS1)
        //C     .          WRITE (LUN,210,ERR=13) X0, Y0, X(N1), Y(N1)
      }
      if (n1 == n0bak) {
        cnstr = false;
      }
      //C
      //C   Bottom of loops.
      //C
      if (lp != lpl) {
        goto statement_6;
      }
      n0bak = n0;
    }
  }
  if (pass1) {
    //C
    //C End of first pass:  paint the path and change to dashed
    //C   lines for subsequent drawing.  Since the scale factors
    //C   are applied to everything, the dash length must be
    //C   specified in world coordinates.
    //C
    pass1 = false;
    //C       WRITE (LUN,150,ERR=13)
    t = dashl * 2.0f / (sfx + sfy);
    //C       WRITE (LUN,220,ERR=13) T
    goto statement_4;
  }
  //C
  //C Paint the path and restore the saved graphics state (with
  //C   no clip path).
  //C
  //C       WRITE (LUN,150,ERR=13)
  //C       WRITE (LUN,230,ERR=13)
  if (numbr) {
    //C
    //C Nodes in the window are to be labeled with their indexes.
    //C   Convert FSIZN from points to world coordinates, and
    //C   output the commands to select a font and scale it.
    //C
    t = fsizn * 2.0f / (sfx + sfy);
    //C       WRITE (LUN,240,ERR=13) T
    //C
    //C   Loop on nodes N0 with coordinates (X0,Y0).
    //C
    FEM_DO_SAFE(n0, 1, n) {
      x0 = x(n0);
      y0 = y(n0);
      if (x0 < wx1 || x0 > wx2 || y0 < wy1 || y0 > wy2) {
        goto statement_9;
      }
      //C
      //C   Move to (X0,Y0), and draw the label N0.  The first char-
      //C     acter will have its lower left corner about one
      //C     character width to the right of the nodal position.
      //C
      //C       WRITE (LUN,180,ERR=13) X0, Y0
      //C       WRITE (LUN,250,ERR=13) N0
      statement_9:;
    }
  }
  //C
  //C Convert FSIZT from points to world coordinates, and output
  //C   the commands to select a font and scale it.
  //C
  t = fsizt * 2.0f / (sfx + sfy);
  //C       WRITE (LUN,240,ERR=13) T
  //C
  //C Display TITLE centered above the plot:
  //C
  y0 = wy2 + 3.0f * t;
  //C       WRITE (LUN,260,ERR=13) TITLE, (WX1+WX2)/2.0, Y0
  //C       WRITE (LUN,270,ERR=13) TITLE
  if (annot) {
    //C
    //C Display the window extrema below the plot.
    //C
    x0 = wx1;
    y0 = wy1 - 100.0f / (sfx + sfy);
    //C       WRITE (LUN,180,ERR=13) X0, Y0
    //C       WRITE (LUN,280,ERR=13) WX1, WX2
    y0 = y0 - 2.0f * t;
    //C       WRITE (LUN,290,ERR=13) X0, Y0, WY1, WY2
  }
  //C
  //C Paint the path and output the showpage command and
  //C   end-of-file indicator.
  //C
  //C       WRITE (LUN,300,ERR=13)
  //C
  //C HP's interpreters require a one-byte End-of-PostScript-Job
  //C   indicator (to eliminate a timeout error message):
  //C   ASCII 4.
  //C
  //C       WRITE (LUN,310,ERR=13) CHAR(4)
  //C
  //C No error encountered.
  //C
  ier = 0;
  return;
  //C
  //C Invalid input parameter.
  //C
  statement_11:
  ier = 1;
  return;
  //C
  //C DX or DY is not positive.
  //C
  statement_12:
  ier = 2;
  return;
  //C
  //C Error writing to unit LUN.
  //C
  ier = 3;

  (void)tx;
  (void)ty;
  (void)cnstr;
}

struct trprnt_save
{
  int nlmax;
  int nmax;

  trprnt_save() :
    nlmax(fem::int0),
    nmax(fem::int0)
  {}
};

void
trprnt(
  common& cmn,
  int const& /* ncc */,
  arr_cref<int> /* lcc */,
  int const& n,
  arr_cref<double> /* x */,
  arr_cref<double> /* y */,
  arr_cref<int> list,
  arr_cref<int> lptr,
  arr_cref<int> lend,
  int const& lout,
  bool const& prntx)
{
  FEM_CMN_SVE(trprnt);
  list(dimension(star));
  lptr(dimension(star));
  lend(dimension(n));
  int& nlmax = sve.nlmax;
  int& nmax = sve.nmax;
  if (is_called_first_time) {
    nmax = 9999;
    nlmax = 60;
  }
  int nn = fem::int0;
  int lun = fem::int0;
  int nl = fem::int0;
  int nb = fem::int0;
  int node = fem::int0;
  int lpl = fem::int0;
  int lp = fem::int0;
  int k = fem::int0;
  int nd = fem::int0;
  arr_1d<100, int> nabor(fem::fill0);
  int inc = fem::int0;
  int nt = fem::int0;
  int na = fem::int0;
  //C
  //C***********************************************************
  //C
  //C                                               From TRIPACK
  //C                                            Robert J. Renka
  //C                                  Dept. of Computer Science
  //C                                       Univ. of North Texas
  //C                                           renka@cs.unt.edu
  //C                                                   07/30/98
  //C
  //C   Given a triangulation of a set of points in the plane,
  //C this subroutine prints the adjacency lists and, option-
  //C ally, the nodal coordinates on logical unit LOUT.  The
  //C list of neighbors of a boundary node is followed by index
  //C 0.  The numbers of boundary nodes, triangles, and arcs,
  //C and the constraint curve starting indexes, if any, are
  //C also printed.
  //C
  //C On input:
  //C
  //C       NCC = Number of constraints.
  //C
  //C       LCC = List of constraint curve starting indexes (or
  //C             dummy array of length 1 if NCC = 0).
  //C
  //C       N = Number of nodes in the triangulation.
  //C           3 .LE. N .LE. 9999.
  //C
  //C       X,Y = Arrays of length N containing the coordinates
  //C             of the nodes in the triangulation -- not used
  //C             unless PRNTX = TRUE.
  //C
  //C       LIST,LPTR,LEND = Data structure defining the trian-
  //C                        gulation.  Refer to Subroutine
  //C                        TRMESH.
  //C
  //C       LOUT = Logical unit number for output.  0 .LE. LOUT
  //C              .LE. 99.  Output is printed on unit 6 if LOUT
  //C              is outside its valid range on input.
  //C
  //C       PRNTX = Logical variable with value TRUE if and only
  //C               if X and Y are to be printed (to 6 decimal
  //C               places).
  //C
  //C None of the parameters are altered by this routine.
  //C
  //C Modules required by TRPRNT:  None
  //C
  //C***********************************************************
  //C
  nn = n;
  lun = lout;
  if (lun < 0 || lun > 99) {
    lun = 6;
  }
  //C
  //C Print a heading and test the range of N.
  //C
  //C       WRITE (LUN,100) NN
  if (nn < 3 || nn > nmax) {
    //C
    //C N is outside its valid range.
    //C
    //C       WRITE (LUN,110)
    goto statement_5;
  }
  //C
  //C Initialize NL (the number of lines printed on the current
  //C   page) and NB (the number of boundary nodes encountered).
  //C
  nl = 6;
  nb = 0;
  if (!prntx) {
    //C
    //C Print LIST only.  K is the number of neighbors of NODE
    //C   which are stored in NABOR.
    //C
    //C       WRITE (LUN,101)
    FEM_DO_SAFE(node, 1, nn) {
      lpl = lend(node);
      lp = lpl;
      k = 0;
      //C
      statement_1:
      k++;
      lp = lptr(lp);
      nd = list(lp);
      nabor(k) = nd;
      if (lp != lpl) {
        goto statement_1;
      }
      if (nd <= 0) {
        //C
        //C   NODE is a boundary node.  Correct the sign of the last
        //C     neighbor, add 0 to the end of the list, and increment
        //C     NB.
        //C
        nabor(k) = -nd;
        k++;
        nabor(k) = 0;
        nb++;
      }
      //C
      //C   Increment NL and print the list of neighbors.
      //C
      inc = (k - 1) / 14 + 2;
      nl += inc;
      if (nl > nlmax) {
        //C       WRITE (LUN,106)
        nl = inc;
      }
      //C       WRITE (LUN,103) NODE, (NABOR(I), I = 1,K)
      //C          IF (K .NE. 14) WRITE (LUN,105)
    }
  }
  else {
    //C
    //C Print X, Y, and LIST.
    //C
    //C       WRITE (LUN,102)
    FEM_DO_SAFE(node, 1, nn) {
      lpl = lend(node);
      lp = lpl;
      k = 0;
      statement_3:
      k++;
      lp = lptr(lp);
      nd = list(lp);
      nabor(k) = nd;
      if (lp != lpl) {
        goto statement_3;
      }
      if (nd <= 0) {
        //C
        //C   NODE is a boundary node.
        //C
        nabor(k) = -nd;
        k++;
        nabor(k) = 0;
        nb++;
      }
      //C
      //C   Increment NL and print X, Y, and NABOR.
      //C
      inc = (k - 1) / 8 + 2;
      nl += inc;
      if (nl > nlmax) {
        //C       WRITE (LUN,106)
        nl = inc;
      }
      //C       WRITE (LUN,104) NODE, X(NODE), Y(NODE),
      //C                                                                        .                    (NABOR(I), I = 1,K)
      //C          IF (K .NE. 8) WRITE (LUN,105)
    }
  }
  //C
  //C Print NB, NA, and NT (boundary nodes, arcs, and
  //C   triangles).
  //C
  nt = 2 * nn - nb - 2;
  na = nt + nn - 1;
  //C      IF (NL .GT. NLMAX-6) WRITE (LUN,106)
  //C       WRITE (LUN,107) NB, NA, NT
  //C
  //C Print NCC and LCC.
  //C
  statement_5:;
  //C     WRITE (LUN,108) NCC
  //C      IF (NCC .GT. 0) WRITE (LUN,109) (LCC(I), I = 1,NCC)
  //C
  //C Print formats:
  //C

  (void)na;
}

} // namespace akima


// C-interface based on fortran code:

void sdsf3p_(int *MD, int *NDP, double *XD, double *YD, double *ZD, int *NXI,
     double *XI, int *NYI, double *YI, double *ZI, int *IER, double *WK,
     int *IWK, bool *EXTRPI, int *NEAR, int *NEXT, double *DIST)
{
	const int argc = 1;
	const char *argv[] = { "flowed" };
	akima::common CMN(argc, argv);
	akima::sdsf3p( CMN, *MD, *NDP,
		akima::arr_ref<double>(*XD),
		akima::arr_ref<double>(*YD),
		akima::arr_cref<double>(*ZD),
		*NXI, akima::arr_cref<double>(*XI),
		*NYI, akima::arr_cref<double>(*YI), akima::arr_ref<double, 2>(*ZI),
		*IER,
		akima::arr_ref<double, 2>(*WK),
		akima::arr_ref<int, 2>(*IWK),
		akima::arr_ref<bool, 2>(*EXTRPI),
		akima::arr_ref<int>(*NEAR),
		akima::arr_ref<int>(*NEXT),
		akima::arr_ref<double>(*DIST) );
}
