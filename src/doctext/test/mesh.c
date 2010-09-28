#ifdef PETSC_RCS_HEADER
static char vcid[] = "$Id: mesh.c,v 1.1.1.1 2000-01-05 16:39:19 gropp Exp $";
#endif


/*
     Defines the interface to the grid functions
*/


#include "src/gvec/mesh/meshimpl.h"    /*I "mesh.h" I*/


#undef  __FUNC__
#define __FUNC__ "MeshDestroy"
/*@
  MeshDestroy - Destroys a mesh object.


  Collective on Mesh


  Input Parameter:
. mesh - The mesh


  Level: beginner


.keywords: mesh, destroy
.seealso MeshCreate()
@*/
int MeshDestroy(Mesh mesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  if (--mesh->refct > 0)
    PetscFunctionReturn(0);
  ierr = (*mesh->ops->destroy)(mesh);                                                                    CHKERRQ(ierr);
  PLogObjectDestroy(mesh);
  PetscHeaderDestroy(mesh); 
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshView"
/*@
  MeshView - Views a mesh object.


  Collective on Mesh


  Input Parameters:
+ mesh   - The mesh
- viewer - The viewer with which to view the mesh


  Level: beginner


.keywords: mesh, view
.seealso: MeshDestroy(), ViewerDrawOpen(), ViewerPoumOpen()
@*/
int MeshView(Mesh mesh, Viewer viewer)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  if (viewer == PETSC_NULL)
    viewer = VIEWER_STDOUT_SELF;
  else
    PetscValidHeaderSpecific(viewer, VIEWER_COOKIE);
  ierr = (*mesh->ops->view)(mesh, viewer);                                                               CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


/*MC
  MeshRegister - Adds a creation method to the mesh package.


  Synopsis:


  MeshRegister(char *name, char *path, char *func_name, int (*create_func)(Mesh, ParameterDict))


  Not Collective


  Input Parameters:
+ name        - The name of a new user-defined creation routine
. path        - The path (either absolute or relative) of the library containing this routine
. func_name   - The name of routine to create method context
- create_func - The creation routine itself


  Notes:
  MeshRegister() may be called multiple times to add several user-defined meshes.


  If dynamic libraries are used, then the fourth input argument (create_func) is ignored.


  Sample usage:
.vb
  MeshRegister("my_mesh", /home/username/my_lib/lib/libO/solaris/mylib.a, "MyFunc", MyFunc);
.ve


  Then, your mesh type can be chosen with the procedural interface via
$     MeshSetType(vec, "my_mesh")
  or at runtime via the option
$     -mesh_type my_mesh


  Level: advanced


  $PETSC_ARCH and $BOPT occuring in pathname will be replaced with appropriate values.


.keywords: mesh, register
.seealso: MeshRegisterAll(), MeshRegisterDestroy()
M*/
#undef __FUNC__  
#define __FUNC__ "MeshRegister_Private"
int MeshRegister_Private(const char sname[], const char path[], const char name[], int (*function)(Mesh, ParameterDict))
{
  char fullname[256];
  int  ierr;


  PetscFunctionBegin;
  ierr = PetscStrcpy(fullname, path);                                                                    CHKERRQ(ierr);
  ierr = PetscStrcat(fullname, ":");                                                                     CHKERRQ(ierr);
  ierr = PetscStrcat(fullname, name);                                                                    CHKERRQ(ierr);
  ierr = FListAdd_Private(&MeshList, sname, fullname, (int (*)(void*)) function);                        CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


/*MC
  MeshSerializeRegister - Adds a serialization method to the mesh package.


  Synopsis:


  MeshSerializeRegister(char *serialize_name, char *path, char *serialize_func_name,
                        int (*serialize_func)(MPI_Comm, Mesh *, Viewer, PetscBool))


  Not Collective


  Input Parameters:
+ serialize_name      - The name of a new user-defined serialization routine
. path                - The path (either absolute or relative) of the library containing this routine
. serialize_func_name - The name of routine to create method context
- serialize_func      - The serialization routine itself


  Notes:
  MeshSerializeRegister() may be called multiple times to add several user-defined solvers.


  If dynamic libraries are used, then the fourth input argument (routine_create) is ignored.


  Sample usage:
.vb
  MeshSerializeRegister("my_store", /home/username/my_lib/lib/libO/solaris/mylib.a, "MyStoreFunc", MyStoreFunc);
.ve


  Then, your serialization can be chosen with the procedural interface via
$     MeshSetSerializeType(vec, "my_store")
  or at runtime via the option
$     -mesh_serialize_type my_store


  Level: advanced


  $PETSC_ARCH and $BOPT occuring in pathname will be replaced with appropriate values.


.keywords: mesh, register
.seealso: MeshSerializeRegisterAll(), MeshSerializeRegisterDestroy()
M*/
#undef __FUNC__  
#define __FUNC__ "MeshSerializeRegister_Private"
int MeshSerializeRegister_Private(const char sname[], const char path[], const char name[],
                                  int (*function)(MPI_Comm, Mesh *, Viewer, PetscBool))
{
  char fullname[256];
  int  ierr;


  PetscFunctionBegin;
  ierr = PetscStrcpy(fullname, path);                                                                    CHKERRQ(ierr);
  ierr = PetscStrcat(fullname, ":");                                                                     CHKERRQ(ierr);
  ierr = PetscStrcat(fullname, name);                                                                    CHKERRQ(ierr);
  ierr = FListAdd_Private(&MeshSerializeList, sname, fullname, (int (*)(void*)) function);               CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
 
#undef  __FUNC__
#define __FUNC__ "MeshCoarsen"
/*@
  MeshCoarsen - Coarsen a mesh based on area constraints.


  Collective on Mesh


  Input Parameters:
+ mesh    - The initial mesh
- area    - A function which gives an area constraint when evaluated inside an element


  Output Parameter:
. newmesh - The coarse mesh


  Note:
  If PETSC_NULL is used for the 'area' argument, then the mesh consisting only of vertices
  is returned.


  Level: beginner


.keywords: grid, mesh, coarsening
.seealso: MeshRefine(), MeshDestroy()
@*/
int MeshCoarsen(Mesh mesh, PointFunction area, Mesh *newmesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidPointer(newmesh);
  if (!mesh->ops->coarsen)
    SETERRQ(PETSC_ERR_SUP, 0, "MeshCoarsen");
  ierr = (*mesh->ops->coarsen)(mesh, area, newmesh);                                                     CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
 
#undef  __FUNC__
#define __FUNC__ "MeshRefine"
/*@
  MeshRefine - Refine a mesh based on area constraints.


  Input Parameters:
+ mesh    - The initial mesh
- area    - A function which gives an area constraint when evaluated inside an element


  Output Parameter:
. newmesh - The refined mesh


  Level: beginner


.keywords: grid, mesh, refinement
.seealso: MeshCoarsen(), MeshDestroy()
@*/
int MeshRefine(Mesh mesh, PointFunction area, Mesh *newmesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidPointer(newmesh);
  if (!mesh->ops->refine)
    SETERRQ(PETSC_ERR_SUP, 0, "MeshRefine");
  ierr = (*mesh->ops->refine)(mesh, area, newmesh);                                                      CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshReform"
/*@
  MeshReform - Reform a mesh using the original boundary.


  Collective on Mesh


  Input Parameters:
+ mesh    - The previous mesh
. refine  - Flag indicating whether to refine or recreate the mesh
. area    - A function which gives an area constraint when evaluated inside an element
- newBd   - Flag to determine whether the boundary should be generated (PETSC_TRUE) or taken from storage


  Output Parameter:
. newMesh - The new mesh


  Level: beginner


.keywords: mesh, refinement, reform
.seealso: MeshReform(), MeshSetBounday()
@*/
int MeshReform(Mesh mesh, PetscBool refine, PointFunction area, PetscBool newBd, Mesh *newMesh)
{
  void *tempCtx;
  int   ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidPointer(newMesh);
  PLogEventBegin(MESH_Reform, mesh, 0, 0, 0);
  /* Save the context for the new mesh */
  ierr = MeshGetUserContext(mesh, &tempCtx);                                                             CHKERRQ(ierr);
  ierr = (*mesh->ops->reform)(mesh, refine, area, newBd, newMesh);                                       CHKERRQ(ierr);


  /* Setup moving mesh support */
  (*newMesh)->movingMesh              = mesh->movingMesh;
  (*newMesh)->meshVelocityMethod      = mesh->meshVelocityMethod;
  (*newMesh)->meshAccelerationMethod  = mesh->meshAccelerationMethod;
  (*newMesh)->movingMeshViewer        = mesh->movingMeshViewer;
  if (mesh->movingMeshViewer != PETSC_NULL)
    PetscObjectReference((PetscObject)  mesh->movingMeshViewer);
  (*newMesh)->movingMeshViewerCaption = mesh->movingMeshViewerCaption;
  (*newMesh)->movingMeshCtx           = mesh->movingMeshCtx;
  if ((*newMesh)->movingMesh) {
    ierr = MeshSetupMovement(*newMesh);                                                                  CHKERRQ(ierr);
  }
  ierr = GridDuplicateBC(mesh->meshVelocityGrid, (*newMesh)->meshVelocityGrid);                          CHKERRQ(ierr);
  ierr = GridDuplicateBC(mesh->meshAccelerationGrid, (*newMesh)->meshAccelerationGrid);                  CHKERRQ(ierr);


  /* Copy over all other information */
  ierr = MeshSetUserContext(*newMesh,  tempCtx);                                                         CHKERRQ(ierr);
  (*newMesh)->highlightElement = mesh->highlightElement;
  PLogEventEnd(MESH_Reform, mesh, 0, 0, 0);


  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshResetNodes"
/*@
  MeshResetNodes - Using the vertex and edge structure, move auxilliary nodes back to
  their default positions.


  Input Parameters:
+ mesh    - The mesh
- resetBd - The flag which indicates whether boundaries should also be reset


  Note:
  This function was developed to allow midnodes to be moved back onto the edges
  with which they were associated. Midnodes on edges between nodes on the same
  boundary are only reset if resetBd == PETSC_TRUE (to allow for curved boundaries).


  Level: advanced


.keywords: mesh, reset, node
.seealso: MeshReform(), MeshSetBounday()
@*/
int MeshResetNodes(Mesh mesh, PetscBool resetBd)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  ierr = (*mesh->ops->resetnodes)(mesh, resetBd);                                                        CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshCopy"
/*@
  MeshCopy - Copies all the data from one mesh to another.


  Collective on Mesh


  Input Parameter:
. mesh    - The mesh


  Output Parameter:
. newmesh - The identical new mesh


  Level: beginner


.keywords: Mesh, copy
.seealso: MeshDuplicate()
@*/
int MeshCopy(Mesh mesh, Mesh newmesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh,    MESH_COOKIE);
  PetscValidHeaderSpecific(newmesh, MESH_COOKIE);
  PetscFunctionBegin;
  if (!mesh->ops->copy)
    SETERRQ(PETSC_ERR_SUP, 0, "MeshCopy");
  ierr = (*mesh->ops->copy)(mesh, newmesh);                                                              CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshDuplicate"
/*@
  MeshDuplicate - Duplicates the mesh structure.


  Collective on Mesh


  Input Parameter:
. mesh    - The mesh


  Output Parameter:
. newmesh - The duplicate mesh


  Level: beginner


.keywords: mesh, duplicate, copy
.seealso: MeshCopy()
@*/
int MeshDuplicate(Mesh mesh, Mesh *newmesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidPointer(newmesh);
  if (!mesh->ops->duplicate)
    SETERRQ(PETSC_ERR_SUP, 0, "MeshDuplicate");
  ierr = (*mesh->ops->duplicate)(mesh, newmesh);                                                         CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetUp"
/*@
  MeshSetUp - Set up any required internal data structures for a mesh.


  Collective on Mesh


  Input Parameter:
. mesh - The mesh


  Level: beginner


.keywords: mesh, setup
.seealso: MeshDestroy()
@*/
int MeshSetUp(Mesh mesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  if (mesh->setupcalled)
    PetscFunctionReturn(0);
  if (mesh->ops->setup) {
    ierr = (*mesh->ops->setup)(mesh);                                                                    CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshCreateGrid"
/*@
  MeshCreateGrid - Creates a grid context from the underlying mesh information


  Collective on Mesh


  Input Parameters:
+ mesh   - The mesh 
. fields - The number of fields
. dtypes - The discretization type for each field, for example DISCRETIZATION_LINEAR
- comp   - The number of components for each field


  Output Parameter:
. grid - The grid


  Level: intermediate


.keywords: mesh, grid
.seealso: MeshDestroy()
@*/
int MeshCreateGrid(Mesh mesh, int fields, DiscretizationType *dtypes, int *comp, Grid *grid)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidPointer(dtypes);
  PetscValidIntPointer(comp);
  PetscValidPointer(grid);
  if (!mesh->ops->creategrid)
    SETERRQ(PETSC_ERR_SUP, 0, "MeshCreateGrid: Not supported by this Mesh object");
  ierr = (*mesh->ops->creategrid)(mesh, fields, dtypes, comp, grid);                                     CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshCreateLocalCSR"
/*@
  MeshCreateLocalCSR - Returns the local mesh in CSR format. All off
  processor edges are ignored, and only the symmetric part of the matrix
  is stored if symmetric is true.


  Not collective


  Input Parameters:
+ mesh          - The mesh
. numBC         - [Optional] The number of constrained nodes to be eliminated
. bcNodes       - [Optional] The numbers of constrained nodes
- symmetric     - Whether to return the entire adjacency list


  Output Parameters:
+ numVertices   - The number of vertices in the graph
. numEdges      - The number of edges in the graph.
. vertOffsets   - List of offsets into the neighbor array for each vertex
- vertNeighbors - List of vertex neighbors


  Level: advanced


.keywords: mesh, partition, CSR
.seealso: MeshDestroyLocalCSR(), MeshPartition()
@*/
int MeshCreateLocalCSR(Mesh mesh, int *numVertices, int *numEdges, int **vertOffsets, int **vertNeighbors,
                       int numBC, int *bcNodes, PetscBool symmetric)
{
  int num;
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(numVertices);
  PetscValidIntPointer(numEdges);
  PetscValidIntPointer(vertOffsets);
  PetscValidIntPointer(vertNeighbors);
  if (numBC > 0) {
    PetscValidIntPointer(bcNodes);
    num = numBC;
  } else {
    num = 0;
  }
  PLogEventBegin(MESH_CreateLocalCSR, mesh, 0, 0, 0);
  ierr = (*mesh->ops->createlocalcsr)(mesh, numVertices, numEdges, vertOffsets, vertNeighbors, num, bcNodes, symmetric);
  CHKERRQ(ierr);
  PLogEventEnd(MESH_CreateLocalCSR, mesh, 0, 0, 0);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshDestroyLocalCSR"
/*@
  MeshDestroyLocalCSR - Destroys the local mesh in CSR format


  Not collective


  Input Parameters:
+ mesh          - The mesh
. vertOffsets   - List of offsets into the neighbor array for each element
- vertNeighbors - List of element neighbors


  Level: advanced


.keywords: mesh, partition
.seealso: MeshCreateLocalCSR(), MeshPartition()
@*/
int MeshDestroyLocalCSR(Mesh mesh, int *vertOffsets, int *vertNeighbors)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  ierr = PetscFree(vertOffsets);                                                                         CHKERRQ(ierr);
  ierr = PetscFree(vertNeighbors);                                                                       CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshCreateDualCSR"
/*@
  MeshCreateDualCSR - Returns the dual of the mesh in distributed CSR format


  Collective on Mesh


  Input Parameter:
. mesh          - The mesh


  Output Parameters:
+ elemOffsets   - List of offsets into the neighbor array for each element
. elemNeighbors - List of element neighbors
. edgeWeights   - [Optional] List of edge weights for the dual graph
- weight        - [Optional] Weight for each edge


  Level: advanced


.keywords: mesh, partition, dual
.seealso: MeshDestroyDualCSR(), MeshPartition()
@*/
int MeshCreateDualCSR(Mesh mesh, int **elemOffsets, int **elemNeighbors, int **edgeWeights, int weight)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(elemOffsets);
  PetscValidIntPointer(elemNeighbors);
  if (weight != 0)
    PetscValidIntPointer(edgeWeights);
  ierr = (*mesh->ops->createdualcsr)(mesh, elemOffsets, elemNeighbors, edgeWeights, weight);             CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshDestroyDualCSR"
/*@
  MeshDestroyDualCSR - Destroys the dual of the mesh in distributed CSR format


  Not collective


  Input Parameters:
+ mesh          - The mesh
. elemOffsets   - List of offsets into the neighbor array for each element
. elemNeighbors - List of element neighbors
- edgeWeights   - List of edge weights for the dual graph


  Level: advanced


.keywords: mesh, partition, dual
.seealso: MeshCreateDualCSR(), MeshPartition()
@*/
int MeshDestroyDualCSR(Mesh mesh, int *elemOffsets, int *elemNeighbors, int *edgeWeights)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  ierr = PetscFree(elemOffsets);                                                                         CHKERRQ(ierr);
  ierr = PetscFree(elemNeighbors);                                                                       CHKERRQ(ierr);
  if (edgeWeights != PETSC_NULL) {
    ierr = PetscFree(edgeWeights);                                                                       CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshPartition"
/*@
  MeshPartition - Partitions the mesh among the processors


  Collective on Mesh


  Input Parameter:
. mesh - the mesh


  Level: beginner


.keywords: mesh, partition
.seealso: MeshGetBoundaryStart()
@*/
int MeshPartition(Mesh mesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PLogEventBegin(MESH_Partition, mesh, 0, 0, 0);
  ierr = (*mesh->ops->partition)(mesh);                                                                  CHKERRQ(ierr);
  PLogEventEnd(MESH_Partition, mesh, 0, 0, 0);
  ierr = MeshUpdateBoundingBox(mesh);                                                                    CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetPartition"
/*@
  MeshGetPartition - Returns the mesh partition object.


  Not collective


  Input Parameter:
. mesh - The mesh


  Output Parameter:
. part - The partition


  Level: intermediate


.keywords: mesh, partition, get
.seealso: GridGetMesh()
@*/
int MeshGetPartition(Mesh mesh, Partition *part)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidPointer(part);
  *part = mesh->part;
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetHoleCoords"
/*@
  MeshSetHoleCoords - Sets the coordinates of holes in the mesh


  Collective on Mesh


  Input Parameter:
+ mesh   - The mesh
. num    - The number of holes
- coords - The coordinates


  Level: advanced


.keywords: mesh, hole, coordinates
.seealso: MeshGetBoundaryStart()
@*/
int MeshSetHoleCoords(Mesh mesh, int num, Vec coords)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh,   MESH_COOKIE);
  PetscValidHeaderSpecific(coords, VEC_COOKIE);
  ierr = (*mesh->ops->setholecoords)(mesh, num, coords);                                                 CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetNodeCoords"
/*@
  MeshGetNodeCoords - Retrieves the coordinates of a selected node


  Input Parameters:
+ mesh  - The mesh
- node  - The canonical node number


  Output Parameters:
. x,y,z - The coordinates


  Level: intermediate


.keywords: mesh, hole, coordinates
.seealso: MeshGetBoundaryStart()
@*/
int MeshGetNodeCoords(Mesh mesh, int node, double *x, double *y, double *z)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidScalarPointer(x);
  PetscValidScalarPointer(y);
  PetscValidScalarPointer(z);
  ierr = (*mesh->ops->getnodecoords)(mesh, node, x, y, z);                                               CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetNodeCoords"
/*@
  MeshSetNodeCoords - Sets the coordinates of a selected node


  Collective on Mesh


  Input Parameters:
+ mesh  - The mesh
. node  - The canonical node number
- x,y,z - The coordinates


  Level: intermediate


.keywords: mesh, node, coordinates
.seealso: MeshGetBoundaryStart()
@*/
int MeshSetNodeCoords(Mesh mesh, int node, double x, double y, double z)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  ierr = (*mesh->ops->setnodecoords)(mesh, node, x, y, z);                                               CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetNodeCoordsSaved"
/*@
  MeshGetNodeCoordsSaved - Retrieves the coordinates of a selected node
  from those saved with MeshSaveMesh().


  Input Parameters:
+ mesh  - The mesh
- node  - The canonical node number


  Output Parameters:
. x,y,z - The coordinates


  Level: intermediate


.keywords: mesh, node, coordinates
.seealso: MeshGetNodeCoords(), MeshSaveMesh()
@*/
int MeshGetNodeCoordsSaved(Mesh mesh, int node, double *x, double *y, double *z)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidScalarPointer(x);
  PetscValidScalarPointer(y);
  PetscValidScalarPointer(z);
  ierr = (*mesh->ops->getnodecoordssaved)(mesh, node, x, y, z);                                          CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetBoundarySize"
/*@
  MeshGetBoundarySize - Returns the number of nodes on the specified boundary.


  Not collective


  Input Parameters:
+ mesh     - The mesh
- boundary - The boundary marker


  Output Parameter:
. size     - The number of nodes on the boundary


  Level: intermediate


.keywords: mesh, boundary
.seealso: MeshGetBoundaryStart()
@*/
int MeshGetBoundarySize(Mesh mesh, int boundary, int *size)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(size);
  ierr = (*mesh->ops->getboundarysize)(mesh, boundary, size);                                            CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetBoundaryIndex"
/*@
  MeshGetBoundaryIndex - Returns the index of the specified boundary.


  Not collective


  Input Parameters:
+ mesh     - The mesh
- boundary - The boundary marker


  Output Parameter:
. index - The index of the boundary


  Level: intermediate


.keywords: mesh, boundary
.seealso: MeshGetBoundarySize()
@*/
int MeshGetBoundaryIndex(Mesh mesh, int boundary, int *index)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(index);
  ierr = (*mesh->ops->getboundaryindex)(mesh, boundary, index);                                          CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetNodeBoundary"
/*@
  MeshGetNodeBoundary - Returns the boundary on which the node lies, or 0 for interior nodes.


  Not collective


  Input Parameters:
+ mesh  - The mesh
- node  - The node


  Output Parameter:
. bd    - The boundary


  Level: intermediate


.keywords: mesh, boundary, node
.seealso: MeshGetBoundarySize()
@*/
int MeshGetNodeBoundary(Mesh mesh, int node, int *bd)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(bd);
  ierr = (*mesh->ops->getnodeboundary)(mesh, node, bd);                                                  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetBoundaryStart"
/*@
  MeshGetBoundaryStart - Retrieves the canonical node number of the first node
  on the specified boundary.


  Not collective


  Input Parameters:
+ mesh     - The mesh
. boundary - The boundary marker
- ghost    - Flag for including ghost nodes


  Output Parameter:
. node     - The canonical node number


  Note:
$ This is typically used as an iteration construct with MeshGetBoundaryNext(),
$ for example:
$
$ MeshGetBoundaryStart(mesh, OUTER_BD, &node, PETSC_FALSE);
$ while (node >= 0) {
$   PetscPrintf(PETSC_COMM_SELF, "I have boundary node %d\n", node);
$   MeshGetBoundaryNext(mesh, OUTER_BD, &node, PETSC_FALSE);
$ }


  Level: intermediate


.keywords: mesh, boundary, iterator
.seealso: MeshGetBoundaryNext()
@*/
int MeshGetBoundaryStart(Mesh mesh, int boundary, int *node, PetscBool ghost)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(node);
  ierr = (*mesh->ops->getboundarystart)(mesh, boundary, node, ghost);                                    CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetBoundaryNext"
/*@
  MeshGetBoundaryNext - Retrieves the canonical node number of the next node
  on the specified boundary, with -1 indicating boundary termination.


  Not collective


  Input Parameters:
+ mesh     - The mesh
. boundary - The boundary marker
- ghost    - Flag for including ghost nodes


  Output Parameter:
. node     - The canonical node number


  Note:
$ This is typically used as an iteration construct with MeshGetBoundaryStart(),
$ for example:
$
$ MeshGetBoundaryStart(mesh, OUTER_BD, &node, PETSC_FALSE);
$ while (node >= 0) {
$   PetscPrintf(PETSC_COMM_SELF, "I have boundary node %d\n", node);
$   MeshGetBoundaryNext(mesh, OUTER_BD, &node, PETSC_FALSE);
$ }


  Also, this only returns nodes which lie in the given domain, thus the above
  loop would run in parallel on all processors, returning different nodes on each.


  Level: intermediate


.keywords: mesh, boundary, iterator
.seealso: MeshGetBoundaryStart()
@*/
int MeshGetBoundaryNext(Mesh mesh, int boundary, int *node, PetscBool ghost)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(node);
  ierr = (*mesh->ops->getboundarynext)(mesh, boundary, node, ghost);                                     CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetActiveBoundary"
/*@
  MeshGetActiveBoundary - Retrieves the boundary marker for the boundary
  last iterated over, or -1 if no iteration has taken place.


  Not collective


  Input Parameter:
. mesh     - The mesh


  Output Parameter:
. boundary - The boundary marker


  Level: advanced


.keywords: grid, boundary, iterator
.seealso: GridGetBoundaryNext(), MeshGetBoundaryStart()
@*/
int MeshGetActiveBoundary(Mesh mesh, int *boundary)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(boundary);
  ierr = (*mesh->ops->getactiveboundary)(mesh, boundary);                                                CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetInfo"
/*@
  MeshGetInfo - Gets some information about the mesh.


  Collective on Mesh


  Input Parameter:
. mesh - The mesh


  Output Parameters:
+ vertices - The number of vertices
. nodes    - The number of nodes
. edges    - The number of edges
- elements - The number of elements


  Level: intermediate


.keywords: mesh
.seealso: MatGetInfo()
@*/
int MeshGetInfo(Mesh mesh, int *vertices, int *nodes, int *edges, int *elements)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  ierr = (*mesh->ops->getinfo)(mesh, vertices, nodes, edges, elements);                                  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetBoundingBox"
/*@
  MeshGetBoundingBox - Returns the bounding box for the mesh


  Not collective


  Input Parameter:
. mesh - The mesh


  Output Parameters:
+ startX, startY, startZ - The lower-left corner of the box
- endX,   endY,   endZ   - The upper-right corner of the box


  Level: intermediate


.keywords: mesh, bounding box
.seealso: MeshUpdateBoundingBox()
@*/
int MeshGetBoundingBox(Mesh mesh, double *startX, double *startY, double *startZ, double *endX, double *endY, double *endZ)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  if (startX != PETSC_NULL) {
    PetscValidPointer(startX);
    *startX = mesh->startX;
  }
  if (startY != PETSC_NULL) {
    PetscValidPointer(startY);
    *startY = mesh->startY;
  }
  if (startZ != PETSC_NULL) {
    PetscValidPointer(startZ);
    *startZ = mesh->startZ;
  }
  if (endX   != PETSC_NULL) {
    PetscValidPointer(endX);
    *endX   = mesh->endX;
  }
  if (endY   != PETSC_NULL) {
    PetscValidPointer(endY);
    *endY   = mesh->endY;
  }
  if (endZ   != PETSC_NULL) {
    PetscValidPointer(endZ);
    *endZ   = mesh->endZ;
  }
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetSupport"
/*@
  MeshGetSupport - This function get the canonical element numbers of all elements
  within the support of a given basis function. A call to MeshRestoreSupport() must
  be made before another call to this function.


  Not collective


  Input Parameters:
+ mesh    - The mesh
. node    - The node containing the basis function
- elem    - An element containing the node, -1 for a search


  Output Parameters:
+ degree  - The degree of the node
- support - A list of the elements in the support


  Note:
  This function currently only returns the elements containing
  any given node, so some basis functions will have a wider
  support than this definition.


  Level: intermediate


.keywords: mesh, support
.seealso: MeshRestoreSupport()
@*/
int MeshGetSupport(Mesh mesh, int node, int elem, int *degree, int **support)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(degree);
  PetscValidPointer(support);
  if (mesh->supportTaken == PETSC_TRUE)
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, 0, "MeshGetSupport: Missing call to MeshRestoreSupport()");
  mesh->supportTaken = PETSC_TRUE;
  ierr = (*mesh->ops->getsupport)(mesh, node, elem, degree, support);                                    CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshRestoreSupport"
/*@
  MeshRestoreSupport - This function returns the support array from MeshGetSupport().


  Not collective


  Input Parameters:
+ mesh    - The mesh
. node    - The node containing the basis function
. elem    - An element containing the node, -1 for a search
. degree  - The degree pf the node
- support - A list of the elements in the support


  Level: intermediate


.keywords: mesh, support
.seealso: MeshGetSupport()
@*/
int MeshRestoreSupport(Mesh mesh, int node, int elem, int *degree, int **support)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  if (*support != mesh->support)
    SETERRQ(PETSC_ERR_ARG_WRONG, 0, "MeshRestoreSupport_Triangular_2D: Invalid support argument");
  mesh->supportTaken = PETSC_FALSE;
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshLocatePoint"
/*@
  MeshLocatePoint - This function returns the element containing
  the given point, or -1 if it is not contained in the local mesh.


  Not collective


  Input Parameters:
+ mesh           - The mesh
- x,y,z          - The node coordinates


  Output Parameter:
. containingElem - An element containing the node, -1 for failure


  Level: beginner


.keywords: mesh, point location
.seealso MeshNearestNode(), MeshGetSupport()
@*/
int MeshLocatePoint(Mesh mesh, double x, double y, double z, int *containingElem)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(containingElem);
  PLogEventBegin(MESH_LocatePoint, mesh, 0, 0, 0);
  ierr = (*mesh->ops->locatepoint)(mesh, x, y, z, containingElem);                                       CHKERRQ(ierr);
  PLogEventEnd(MESH_LocatePoint, mesh, 0, 0, 0);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshNearestNode"
/*@
  MeshNearestNode - This function returns the node nearest to
  the given point, or -1 if the closest node is not contained in
  the local mesh.


  Not collective


  Input Parameters:
+ mesh  - The mesh
- x,y,z - The node coordinates


  Output Parameter:
. node  - The nearest node


  Level: beginner


.keywords: mesh, node, point location
.seealso MeshLocatePoint()
@*/
int MeshNearestNode(Mesh mesh, double x, double y, double z, int *node)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidIntPointer(node);
  ierr = (*mesh->ops->nearestnode)(mesh, x, y, z, node);                                                 CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshHighlightElement"
/*@C MeshHighlightElement
        This function highlights the given element when the mesh is displayed.


  Collective on Mesh


  Input Parameter:
. mesh  - The mesh


  Output Parameter:
. element - The element to highlight


  Level: beginner


.keywords mesh, element
.seealso GridHighlightElement()
@*/
int MeshHighlightElement(Mesh mesh, int element)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  mesh->highlightElement = element;
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetUserContext"
/*@
  MeshSetUserContext - Sets the optional user-defined context for a mesh object.


  Collective on Mesh


  Input Parameters:
+ mesh   - The mesh
- usrCtx - The optional user context


  Level: intermediate


.keywords: mesh, set, context
.seealso: MeshGetUserContext(), GridSetMeshContext()
@*/
int MeshSetUserContext(Mesh mesh, void *usrCtx)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  mesh->usr = usrCtx;
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetUserContext"
/*@
  MeshGetUserContext - Gets the optional user-defined context for a mesh object.


  Not collective


  Input Parameters:
. mesh - The mesh


  Output Parameters:
. usrCtx   - The optional user context


  Level: intermediate


.keywords: mesh, set, context
.seealso: MeshSetUserContext(), GridGetMeshContext()
@*/
int MeshGetUserContext(Mesh mesh, void **usrCtx)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidPointer(usrCtx);
  *usrCtx = mesh->usr;
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetMovement"
/*@
  MeshGetMovement - Returns the status of mesh movement.


  Not collective


  Input Parameter:
. mesh - The mesh


  Output Parameters:
+ flag  - The movement flag
. vtype - The type of mesh velocity calculation, like MESH_LAPLACIAN
. atype - The type of mesh acceleration calculation, like MESH_LAPLACIAN
- ctx   - A user context


  Level: intermediate


.keywords: mesh, movement
.seealso: MeshSetMovement()
@*/
int MeshGetMovement(Mesh mesh, PetscBool *flag, MeshSolveMethod *vtype, MeshSolveMethod *atype, void **ctx)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  if (flag  != PETSC_NULL) {
    PetscValidPointer(flag);
    *flag  = mesh->movingMesh;
  }
  if (vtype != PETSC_NULL) {
    PetscValidPointer(vtype);
    *vtype = mesh->meshVelocityMethod;
  }
  if (atype != PETSC_NULL) {
    PetscValidPointer(atype);
    *atype = mesh->meshAccelerationMethod;
  }
  if (ctx   != PETSC_NULL) {
    PetscValidPointer(ctx);
    *ctx   = mesh->movingMeshCtx;
  }
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetMovement"
/*@
  MeshSetMovement - Turns mesh movement on and off.


  Collective on Mesh


  Input Parameters:
+ mesh  - The mesh
. flag  - Flag to turn movement on or off
. vtype - The type of mesh velocity calculation, like MESH_LAPLACIAN
. atype - The type of mesh acceleration calculation, like MESH_LAPLACIAN
- ctx   - A user context


  Level: intermediate


.keywords: mesh, movement
.seealso: MeshGetMovement()
@*/
int MeshSetMovement(Mesh mesh, PetscBool flag, MeshSolveMethod vtype, MeshSolveMethod atype, void *ctx)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  mesh->movingMesh               = flag;
  if (vtype != PETSC_DECIDE)
    mesh->meshVelocityMethod     = vtype;
  if (atype != PETSC_DECIDE)
    mesh->meshAccelerationMethod = atype;
  if (ctx   != PETSC_NULL)
    mesh->movingMeshCtx          = ctx;
  if (flag == PETSC_TRUE)
  {
    if (mesh->meshVelocityGrid == PETSC_NULL)
      {ierr = MeshSetupMovement(mesh);                                                                   CHKERRQ(ierr);}
  }
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetVelocityGrid"
/*@
  MeshGetVelocityGrid - Returns the grid which defines the mesh velocity.


  Not collective


  Input Parameter:
. mesh    - The mesh


  Output Parameter:
. velGrid - The mesh velocity grid


  Level: intermediate


.keywords: grid, mesh, mesh velocity, movement
.seealso: MeshGetAccelerationGrid(), GridGetMeshMovement()
@*/
int MeshGetVelocityGrid(Mesh mesh, Grid *velGrid)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  *velGrid = mesh->meshVelocityGrid;
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshGetAccelerationGrid"
/*@
  MeshGetAccelerationGrid - Returns the grid which defines the mesh acceleration.


  Not collective


  Input Parameter:
. mesh    - The mesh


  Output Parameter:
. accGrid - The mesh acceleration grid


  Level: intermediate


.keywords: grid, mesh, mesh acceleration, movement
.seealso: MeshGetVelocityGrid(), GridGetMeshMovement()
@*/
int MeshGetAccelerationGrid(Mesh mesh, Grid *accGrid)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  *accGrid = mesh->meshAccelerationGrid;
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetupMovement"
/*@
  MeshSetupMovement - Creates all the necessary structures for
  mesh movement.


  Collective on Mesh


  Input Parameter:
. mesh - The mesh


  Note:
  This function should be called directly before beginning the
  calculation, and after all options have been set. It is
  normally called automatically by the solver.


  Level: advanced


.keywords: mesh, movement
.seealso: MeshGetMovement()
@*/
int MeshSetupMovement(Mesh mesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  ierr = (*mesh->ops->setupmovement)(mesh);                                                              CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshCleanupMovement"
/*@
  MeshCleanupMovement - Destroys all the necessary structures for
  mesh movement.


  Collective on Mesh


  Input Parameter:
. mesh - The mesh


  Note:
  This function should be called directly before MeshDestroy(). It
  is normally called automatically by the solver.


  Level: advanced


.keywords: mesh, movement
.seealso: MeshGetMovement()
@*/
int MeshCleanupMovement(Mesh mesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->nodeVelocities != PETSC_NULL)
    {ierr = GVecDestroy(mesh->nodeVelocities);                                                           CHKERRQ(ierr);}
  if (mesh->nodeVelocitiesGhost != PETSC_NULL)
    {ierr = GVecDestroy(mesh->nodeVelocitiesGhost);                                                      CHKERRQ(ierr);}
  if (mesh->meshVelocityMat != PETSC_NULL)
    {ierr = GMatDestroy(mesh->meshVelocityMat);                                                          CHKERRQ(ierr);}
  if (mesh->meshVelocityRhs != PETSC_NULL)
    {ierr = GVecDestroy(mesh->meshVelocityRhs);                                                          CHKERRQ(ierr);}
  if (mesh->meshVelocitySles != PETSC_NULL)
    {ierr = SLESDestroy(mesh->meshVelocitySles);                                                         CHKERRQ(ierr);}
  if (mesh->meshVelocityGrid != PETSC_NULL)
  {
    ierr = GridFinalizeBC(mesh->meshVelocityGrid);                                                       CHKERRQ(ierr);
    ierr = GridDestroy(mesh->meshVelocityGrid);                                                          CHKERRQ(ierr);
  }


  if (mesh->nodeAccelerations != PETSC_NULL)
    {ierr = GVecDestroy(mesh->nodeAccelerations);                                                        CHKERRQ(ierr);}
  if (mesh->nodeAccelerationsGhost != PETSC_NULL)
    {ierr = GVecDestroy(mesh->nodeAccelerationsGhost);                                                   CHKERRQ(ierr);}
  if (mesh->meshAccelerationMat != PETSC_NULL)
    {ierr = GMatDestroy(mesh->meshAccelerationMat);                                                      CHKERRQ(ierr);}
  if (mesh->meshAccelerationRhs != PETSC_NULL)
    {ierr = GVecDestroy(mesh->meshAccelerationRhs);                                                      CHKERRQ(ierr);}
  if (mesh->meshAccelerationSles != PETSC_NULL)
    {ierr = SLESDestroy(mesh->meshAccelerationSles);                                                     CHKERRQ(ierr);}
  if (mesh->meshAccelerationGrid != PETSC_NULL)
  {
    ierr = GridFinalizeBC(mesh->meshAccelerationGrid);                                                   CHKERRQ(ierr);
    ierr = GridDestroy(mesh->meshAccelerationGrid);                                                      CHKERRQ(ierr);
  }


  if (mesh->movingMeshViewer != PETSC_NULL)
    {ierr = ViewerDestroy(mesh->movingMeshViewer);                                                       CHKERRQ(ierr);}
  ierr = (*mesh->ops->cleanupmovement)(mesh);                                                            CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshCalcNodeVelocities"
/*@
  MeshCalcNodeVelocities - Recalculates the node velocities.


  Collective on Mesh


  Input Parameters:
+ mesh - The mesh
- flag - Flag for reuse of old operator in calculation


  Level: advanced


.keywords: mesh, movement
.seealso: MeshCalcNodeAccelerations()
@*/
int MeshCalcNodeVelocities(Mesh mesh, PetscBool flag)
{
  SLES   sles;
  GMat   mat;
  GVec   vec;
  GVec   rhs;
  Scalar zero = 0.0;
  int    its;
  int    ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->movingMesh == PETSC_FALSE)
    PetscFunctionReturn(0);
  PLogEventBegin(MESH_CalcNodeVelocities, mesh, 0, 0, 0);


  /* Allocation */
  if (mesh->nodeVelocities == PETSC_NULL) {
    ierr = GVecCreate(mesh->meshVelocityGrid, &mesh->nodeVelocities);                                    CHKERRQ(ierr);
    PLogObjectParent(mesh, mesh->nodeVelocities);
  }
  if (mesh->meshVelocityRhs == PETSC_NULL) {
    ierr = GVecCreate(mesh->meshVelocityGrid, &mesh->meshVelocityRhs);                                   CHKERRQ(ierr);
    PLogObjectParent(mesh, mesh->meshVelocityRhs);
  }
  if (mesh->meshVelocityMat == PETSC_NULL) {
    ierr = GMatCreate(mesh->meshVelocityGrid, &mesh->meshVelocityMat);                                   CHKERRQ(ierr);
    PLogObjectParent(mesh, mesh->meshVelocityMat);
  }
  if (mesh->meshVelocitySles == PETSC_NULL) {
    ierr = SLESCreate(mesh->comm, &mesh->meshVelocitySles);                                              CHKERRQ(ierr);
    ierr = SLESAppendOptionsPrefix(mesh->meshVelocitySles, "mesh_vel_");                                 CHKERRQ(ierr);
    ierr = SLESSetFromOptions(mesh->meshVelocitySles);                                                   CHKERRQ(ierr);
    PLogObjectParent(mesh, mesh->meshVelocitySles);
  }
  sles = mesh->meshVelocitySles;
  mat  = mesh->meshVelocityMat;
  vec  = mesh->nodeVelocities;
  rhs  = mesh->meshVelocityRhs;


  /* Form System Matrix */
  if (flag == PETSC_TRUE) {
    ierr = MatZeroEntries(mat);                                                                          CHKERRQ(ierr);
    ierr = GMatEvaluateSystemMatrix(mat, MAT_FINAL_ASSEMBLY, mesh->movingMeshCtx);                       CHKERRQ(ierr);
    ierr = GMatSetBoundary(mat, 1.0, mesh->movingMeshCtx);                                               CHKERRQ(ierr);
    ierr = MatSetOption(mat, MAT_NO_NEW_NONZERO_LOCATIONS);                                              CHKERRQ(ierr);
  }
  /* Form Rhs */
  ierr = VecSet(&zero, rhs);                                                                             CHKERRQ(ierr);
  ierr = GVecEvaluateRhs(rhs, &mesh->movingMeshCtx);                                                     CHKERRQ(ierr);
  ierr = GVecSetBoundary(rhs, mesh->movingMeshCtx);                                                      CHKERRQ(ierr);
  /* Solve system */
  ierr = SLESSetOperators(sles, mat, mat, SAME_NONZERO_PATTERN);                                         CHKERRQ(ierr);
  ierr = SLESSolve(sles, rhs, vec, &its);                                                                CHKERRQ(ierr);
  /* Copy values into the local vector */
  ierr = GridGlobalToLocal(mesh->meshVelocityGrid, INSERT_VALUES, vec);                                  CHKERRQ(ierr);
  PLogEventEnd(MESH_CalcNodeVelocities, mesh, 0, 0, 0);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshCalcNodeAccelerations"
/*@
  MeshCalcNodeAccelerations - Recalculates the node accelerations.


  Collective on Mesh


  Input Parameters:
+ mesh - The mesh
- flag - Flag for reuse of old operator in calculation


  Level: advanced


.keywords: mesh, movement
.seealso: MeshCalcNodeVelocities()
@*/
int MeshCalcNodeAccelerations(Mesh mesh, PetscBool flag)
{
  SLES   sles;
  GMat   mat;
  GVec   vec;
  GVec   rhs;
  Scalar zero = 0.0;
  int    its;
  int    ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->movingMesh == PETSC_FALSE)
    PetscFunctionReturn(0);
  PLogEventBegin(MESH_CalcNodeAccelerations, mesh, 0, 0, 0);


  /* Allocation */
  if (mesh->nodeAccelerations == PETSC_NULL) {
    ierr = GVecCreate(mesh->meshAccelerationGrid, &mesh->nodeAccelerations);                             CHKERRQ(ierr);
    PLogObjectParent(mesh, mesh->nodeAccelerations);
  }
  if (mesh->meshAccelerationRhs == PETSC_NULL) {
    ierr = GVecCreate(mesh->meshAccelerationGrid, &mesh->meshAccelerationRhs);                           CHKERRQ(ierr);
    PLogObjectParent(mesh, mesh->meshAccelerationRhs);
  }
  if (mesh->meshAccelerationMat == PETSC_NULL) {
    ierr = GMatCreate(mesh->meshAccelerationGrid, &mesh->meshAccelerationMat);                           CHKERRQ(ierr);
    PLogObjectParent(mesh, mesh->meshAccelerationMat);
  }
  if (mesh->meshAccelerationSles == PETSC_NULL) {
    ierr = SLESCreate(mesh->comm, &mesh->meshAccelerationSles);                                          CHKERRQ(ierr);
    ierr = SLESAppendOptionsPrefix(mesh->meshAccelerationSles, "mesh_accel_");                           CHKERRQ(ierr);
    ierr = SLESSetFromOptions(mesh->meshAccelerationSles);                                               CHKERRQ(ierr);
    PLogObjectParent(mesh, mesh->meshAccelerationSles);
  }
  sles = mesh->meshAccelerationSles;
  mat  = mesh->meshAccelerationMat;
  vec  = mesh->nodeAccelerations;
  rhs  = mesh->meshAccelerationRhs;


  /* Form System Matrix */
  if (flag == PETSC_TRUE) {
    ierr = MatZeroEntries(mat);                                                                          CHKERRQ(ierr);
    ierr = GMatEvaluateSystemMatrix(mat, MAT_FINAL_ASSEMBLY, mesh->movingMeshCtx);                       CHKERRQ(ierr);
    ierr = GMatSetBoundary(mat, 1.0, mesh->movingMeshCtx);                                               CHKERRQ(ierr);
    ierr = MatSetOption(mat, MAT_NO_NEW_NONZERO_LOCATIONS);                                              CHKERRQ(ierr);
  }
  /* Form Rhs */
  ierr = VecSet(&zero, rhs);                                                                             CHKERRQ(ierr);
  ierr = GVecEvaluateRhs(rhs, &mesh->movingMeshCtx);                                                     CHKERRQ(ierr);
  ierr = GVecSetBoundary(rhs, mesh->movingMeshCtx);                                                      CHKERRQ(ierr);
  /* Solve system */
  ierr = SLESSetOperators(sles, mat, mat, SAME_NONZERO_PATTERN);                                         CHKERRQ(ierr);
  ierr = SLESSolve(sles, rhs, vec, &its);                                                                CHKERRQ(ierr);
  /* Copy values into the local vector */
  ierr = GridGlobalToLocal(mesh->meshAccelerationGrid, INSERT_VALUES, vec);                              CHKERRQ(ierr);
  PLogEventEnd(MESH_CalcNodeAccelerations, mesh, 0, 0, 0);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetNodeVelocities"
/*@
  MeshSetNodeVelocities - Allows the user to specify node velocities.


  Collective on Mesh


  Input Parameters:
+ mesh  - The mesh
. func  - A PointFunction defining the node velocities
. alpha - A scalar multiplier
- ctx   - A user context


  Level: advanced


.keywords: mesh, movement
.seealso: MeshSetNodeAccelerations()
@*/
int MeshSetNodeVelocities(Mesh mesh, PointFunction func, Scalar alpha, void *ctx)
{
  int field = 0;
  int ierr;


  PetscFunctionBegin;
  ierr = GVecEvaluateFunction(mesh->nodeVelocities, 1, &field, func, alpha, ctx);                        CHKERRQ(ierr);
  ierr = GridGlobalToLocal(mesh->meshVelocityGrid, INSERT_VALUES, mesh->nodeVelocities);                 CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetNodeAccelerations"
/*@
  MeshSetNodeAccelerations - Allows the user to specify node accelerations.


  Collective on Mesh


  Input Parameters:
+ mesh  - The mesh
. func  - A function defining the node accelerations
. alpha - A scalar multiplier
- ctx   - A user context


  Level: intermediate


.keywords: mesh, movement
.seealso: MeshSetNodeVelocities()
@*/
int MeshSetNodeAccelerations(Mesh mesh, PointFunction func, Scalar alpha, void *ctx)
{
  int field = 0;
  int ierr;


  PetscFunctionBegin;
  ierr = GVecEvaluateFunction(mesh->nodeAccelerations, 1, &field, func, alpha, ctx);                     CHKERRQ(ierr);
  ierr = GridGlobalToLocal(mesh->meshAccelerationGrid, INSERT_VALUES, mesh->nodeAccelerations);          CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetVelocityBC"
/*@C
  MeshSetVelocityBC -   This function sets the boundary conditions to use
  for the mesh velocity calculation.


  Collective on Mesh


  Input Parameters:
+ mesh   - The mesh
. bd     - The marker for the boundary along which conditions are applied
. f      - The function which defines the boundary condition
- reduce - The flag for explicit reduction of the system


  Note:
  This function clears any previous conditions, whereas MeshAddVelocityBC()
  appends the condition.


  Level: intermediate


.keywords mesh, velocity, boundary condition
.seealso MeshAddVelocityBC(), MeshSetAccelerationBC(), GridSetBC(), GridAddBC()
@*/
int MeshSetVelocityBC(Mesh mesh, int bd, PointFunction f, PetscBool reduce)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->movingMesh == PETSC_FALSE)
    PetscFunctionReturn(0);
  if (mesh->meshVelocityGrid == PETSC_NULL)
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, 0, "MeshSetVelocityBC: Velocity grid not created");
  ierr = GridSetBC(mesh->meshVelocityGrid, bd, 0, f, reduce);                                            CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshAddVelocityBC"
/*@C
  MeshAddVelocityBC - This function adds a boundary condition to use
  for the mesh velocity calculation.


  Collective on Mesh


  Input Parameters:
+ mesh   - The mesh
. bd     - The marker for the boundary along which conditions are applied
. f      - The function which defines the boundary condition
- reduce - The flag for explicit reduction of the system


  Note:
  This function appends the condition, whereas MeshSetVelocityBC()
  clears any previous conditions.


  Level: intermediate


.keywords mesh, velocity, boundary condition
.seealso MeshSetVelocityBC(), MeshAddAccelerationBC(), GridSetBC(), GridAddBC()
@*/
int MeshAddVelocityBC(Mesh mesh, int bd, PointFunction f, PetscBool reduce)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->movingMesh == PETSC_FALSE)
    PetscFunctionReturn(0);
  if (mesh->meshVelocityGrid == PETSC_NULL)
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, 0, "MeshAddVelocityBC: Velocity grid not created");
  ierr = GridAddBC(mesh->meshVelocityGrid, bd, 0, f, reduce);                                            CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetVelocityBCContext"
/*@C
  MeshSetVelocityBCContext - This function sets the boundary condition context
  to use for the mesh velocity calculation.


  Collective on Mesh


  Input Parameters:
+ mesh - The mesh
- ctx  - The user context


  Level: intermediate


.keywords mesh, velocity, boundary condition
.seealso MeshAddVelocityBC(), MeshSetVelocityBC()
@*/
int MeshSetVelocityBCContext(Mesh mesh, void *ctx)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->movingMesh == PETSC_FALSE)
    PetscFunctionReturn(0);
  if (mesh->meshVelocityGrid == PETSC_NULL)
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, 0, "MeshSetVelocityBC: Velocity grid not created");
  ierr = GridSetBCContext(mesh->meshVelocityGrid, ctx);                                                  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetAccelerationBC"
/*@C
  MeshSetAccelerationBC - This function sets the boundary condition to use
  for the mesh acceleration calculation.


  Collective on Mesh


  Input Parameters:
+ mesh   - The mesh
. bd     - The marker for the boundary along which conditions are applied
. f      - The function which defines the boundary condition
- reduce - The flag for explicit reduction of the system


  Note:
  This function clears any previous conditions, whereas MeshAddAccelerationBC()
  appends the condition.


  Level: intermediate


.keywords mesh, velocity, boundary condition
.seealso MeshAddAccelerationBC(), MeshSetVelocityBC(), GridSetBC(), GridAddBC()
@*/
int MeshSetAccelerationBC(Mesh mesh, int bd, PointFunction f, PetscBool reduce)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->movingMesh == PETSC_FALSE)
    PetscFunctionReturn(0);
  if (mesh->meshAccelerationGrid == PETSC_NULL)
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, 0, "MeshSetAccelerationBC: Acceleration grid not created");
  ierr = GridSetBC(mesh->meshAccelerationGrid, bd, 0, f, reduce);                                        CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshAddAccelerationBC"
/*@C
  MeshAddAccelerationBC - This function adds a boundary condition to use
  for the mesh acceleration calculation.


  Collective on Mesh


  Input Parameters:
+ mesh   - The mesh
. bd     - The marker for the boundary along which conditions are applied
. f      - The function which defines the boundary condition
- reduce - The flag for explicit reduction of the system


  Note:
  This function appends the condition, whereas MeshSetVelocityBC()
  clears any previous conditions.


  Level: intermediate


.keywords mesh, velocity, boundary condition
.seealso MeshSetAccelerationBC(), MeshAddVelocityBC(), GridSetBC(), GridAddBC()
@*/
int MeshAddAccelerationBC(Mesh mesh, int bd, PointFunction f, PetscBool reduce)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->movingMesh == PETSC_FALSE)
    PetscFunctionReturn(0);
  if (mesh->meshAccelerationGrid == PETSC_NULL)
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, 0, "MeshAddAccelerationBC: Acceleration grid not created");
  ierr = GridAddBC(mesh->meshAccelerationGrid, bd, 0, f, reduce);                                        CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetAccelerationBCContext"
/*@C
  MeshSetAccelerationBCContext - This function sets the boundary condition context
  to use for the mesh acceleration calculation.


  Collective on Mesh


  Input Parameter:
+ mesh - The mesh
- ctx  - The user context


  Level: intermediate


.keywords mesh, velocity, boundary condition
.seealso MeshAddAccelerationBC(), MeshSetAccelerationBC()
@*/
int MeshSetAccelerationBCContext(Mesh mesh, void *ctx)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->movingMesh == PETSC_FALSE)
    PetscFunctionReturn(0);
  if (mesh->meshAccelerationGrid == PETSC_NULL)
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, 0, "MeshSetAccelerationBC: Acceleration grid not created");
  ierr = GridSetBCContext(mesh->meshAccelerationGrid, ctx);                                              CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshUpdateBoundingBox"
/*@
  MeshUpdateBoundingBox - Recalculates the bounding box for the mesh


  Collective on Mesh


  Input Parameter:
. mesh - The mesh


  Note:
  This function is called automatically after the mesh
  is moved, but must be called by the user if coordinates
  are reset by hand.


  Level: advanced


.keywords: mesh, bounding box
.seealso: MeshGetBoundingBox()
@*/
int MeshUpdateBoundingBox(Mesh mesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  if (mesh->isPeriodic == PETSC_TRUE)
    PetscFunctionReturn(0);
  /* Calculate local bounding rectangle */
  ierr = (*mesh->ops->updateboundingbox)(mesh);                                                          CHKERRQ(ierr);
  /* Calculate global bounding rectangle */
  ierr = MPI_Allreduce(&mesh->locStartX, &mesh->startX, 1, MPI_DOUBLE, MPI_MIN, mesh->comm);             CHKERRQ(ierr);
  ierr = MPI_Allreduce(&mesh->locEndX,   &mesh->endX,   1, MPI_DOUBLE, MPI_MAX, mesh->comm);             CHKERRQ(ierr);
  ierr = MPI_Allreduce(&mesh->locStartY, &mesh->startY, 1, MPI_DOUBLE, MPI_MIN, mesh->comm);             CHKERRQ(ierr);
  ierr = MPI_Allreduce(&mesh->locEndY,   &mesh->endY,   1, MPI_DOUBLE, MPI_MAX, mesh->comm);             CHKERRQ(ierr);
  mesh->sizeX = mesh->endX - mesh->startX;
  mesh->sizeY = mesh->endY - mesh->startY;
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshMoveMesh"
/*@
  MeshMoveMesh - Recalculates the node coordinates based on the
  current velocities and accelerations.


  Collective on Mesh


  Input Parameters:
. mesh - The mesh
. dt   - The timestep


  Level: advanced


.keywords: mesh, movement
.seealso: MeshCalcNodeVelocitiess(), MeshCalcNodeAccelerations()
@*/
int MeshMoveMesh(Mesh mesh, double dt)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);


  if (mesh->movingMesh == PETSC_FALSE)
    PetscFunctionReturn(0);
  PLogEventBegin(MESH_MoveMesh, mesh, 0, 0, 0);
  ierr = (*mesh->ops->movemesh)(mesh, dt);                                                               CHKERRQ(ierr);
  PLogEventEnd(MESH_MoveMesh, mesh, 0, 0, 0);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshSaveMesh"
/*@
  MeshSaveMesh - This function saves the mesh coordinates.


  Collective on Mesh


  Input Parameter:
. mesh - The mesh


  Level: advanced


  Note:
  This function is meant to be used in conjuction with MeshMoveMesh(),
  and MeshRestoreMesh() so that the mesh may be moved, checked for
  distortion, and if necessary moved back.


.keywords: mesh, movement
.seealso: MeshMoveMesh(), MeshRestoreMesh()
@*/
int MeshSaveMesh(Mesh mesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  ierr = (*mesh->ops->savemesh)(mesh);                                                                   CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshRestoreMesh"
/*@
  MeshRestoreMesh - This function restores the mesh coordinates which were
  previously saved.


  Collective on Mesh


  Input Parameter:
. mesh - The mesh


  Level: advanced


  Note:
  This function is meant to be used in conjuction with MeshMoveMesh(),
  and MeshRestoreMesh() so that the mesh may be moved, checked for
  distortion, and if necessary moved back.


.keywords: mesh, movement
.seealso: MeshMoveMesh(), MeshSaveMesh()
@*/
int MeshRestoreMesh(Mesh mesh)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  ierr = (*mesh->ops->restoremesh)(mesh);                                                                CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef  __FUNC__
#define __FUNC__ "MeshIsDistorted"
/*@
  MeshIsDistorted - This function checks for more than the maximum
  level of distortion in the mesh. It also returns an error when
  encountering elements with negative area.


  Collective on Mesh


  Input Parameter:
. mesh - The mesh


  Output Parameter:
. flag - Signals a distorted mesh


  Level: intermediate


.keywords: mesh, distortion, movement
.seealso: MeshMoveMesh()
@*/
int MeshIsDistorted(Mesh mesh, PetscBool *flag)
{
  int ierr;


  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidPointer(flag);
  PLogEventBegin(MESH_IsDistorted, mesh, 0, 0, 0);
  /* Do not use CHKERRQ since we want the return value at the top level */
  ierr = (*mesh->ops->isdistorted)(mesh, flag);
  PLogEventEnd(MESH_IsDistorted, mesh, 0, 0, 0);
  PetscFunctionReturn(ierr);
}


#undef  __FUNC__
#define __FUNC__ "MeshSetMovementCaption"
/*@
  MeshSetMovementCaption - Sets the caption of the moving mesh
  viewer. Can be used to provide auxilliary information, e.g. time.


  Collective on Mesh


  Input Parameters:
+ mesh    - The mesh
- caption - The caption


  Level: intermediate


.keywords: mesh, movement
.seealso: MeshMoveMesh()
@*/
int MeshSetMovementCaption(Mesh mesh, char *caption)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mesh, MESH_COOKIE);
  PetscValidPointer(caption);
  mesh->movingMeshViewerCaption = caption;
  PetscFunctionReturn(0);
}


/*------------------------------------------ Periodic Mesh Functions ------------------------------------------------*/
#undef  __FUNC__
#define __FUNC__ "MeshIsPeriodic"
/*@
  MeshIsPeriodic - Returns the periodicity of the mesh.


  Not collective


  Input Parameter:
. mesh - The mesh


  Output Parameters:
+ x    - The flag for periodicity in x (or PETSC_NULL)
. y    - The flag for periodicity in y (or PETSC_NULL)
- z    - The flag for periodicity in z (or PETSC_NULL)


  Level: intermediate


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicX(), MeshPeriodicRelativeX(), MeshPeriodicDiffX()
@*/
int MeshIsPeriodic(Mesh mesh, PetscBool *x, PetscBool *y, PetscBool *z)
{
  PetscFunctionBegin;
  if (x != PETSC_NULL) {
    PetscValidPointer(x);
    *x = mesh->isPeriodicDim[0];
  }
  if (y != PETSC_NULL) {
    PetscValidPointer(y);
    *y = mesh->isPeriodicDim[1];
  }
  if (z != PETSC_NULL) {
    PetscValidPointer(z);
    *z = mesh->isPeriodicDim[2];
  }
  PetscFunctionReturn(0);
}


#ifndef MESH_PERIODIC_OPTIMIZED


#undef  __FUNC__
#define __FUNC__ "MeshPeriodicX"
/*@
  MeshPeriodicX - Returns the value modulo the period of the mesh.


  Not collective


  Input Parameters:
+ mesh - The mesh
- x    - The original coordinate


  Output Parameter:
. ret  - The normalized coordinate


  Level: intermediate


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicY(), MeshPeriodicZ(), MeshPeriodicRelativeX(), MeshPeriodicDiffX()
@*/
double MeshPeriodicX(Mesh mesh, double x)
{
  PetscFunctionBegin;
  if (mesh->isPeriodicDim[0] == PETSC_FALSE)
    PetscFunctionReturn(x);
  if (x - PETSC_MACHINE_EPSILON > mesh->endX) {
    if (x - mesh->sizeX > mesh->endX) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one\n", x);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(x - mesh->sizeX);
  } else if (x + PETSC_MACHINE_EPSILON < mesh->startX) {
    if (x + mesh->sizeX < mesh->startX) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one\n", x);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(x + mesh->sizeX);
  } else {
    PetscFunctionReturn(x);
  }
}


#undef  __FUNC__
#define __FUNC__ "MeshPeriodicY"
/*@
  MeshPeriodicY - Returns the value modulo the period of the mesh.


  Not collective


  Input Parameters:
+ mesh - The mesh
- y    - The original coordinate


  Output Parameter:
. ret  - The normalized coordinate


  Level: intermediate


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicX(), MeshPeriodicZ(), MeshPeriodicRelativeY(), MeshPeriodicDiffY()
@*/
double MeshPeriodicY(Mesh mesh, double y)
{
  PetscFunctionBegin;
  if (mesh->isPeriodicDim[1] == PETSC_FALSE)
    PetscFunctionReturn(y);
  if (y - PETSC_MACHINE_EPSILON > mesh->endY) {
    if (y - mesh->sizeY > mesh->endY) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one\n", y);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(y - mesh->sizeY);
  } else if (y + PETSC_MACHINE_EPSILON < mesh->startY) {
    if (y + mesh->sizeY < mesh->startY) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one\n", y);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(y + mesh->sizeY);
  } else {
    PetscFunctionReturn(y);
  }
}


#undef  __FUNC__
#define __FUNC__ "MeshPeriodicZ"
/*@
  MeshPeriodicZ - Returns the value modulo the period of the mesh.


  Not collective


  Input Parameters:
+ mesh - The mesh
- z    - The original coordinate


  Output Parameter:
. ret  - The normalized coordinate


  Level: intermediate


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicX(), MeshPeriodicY(), MeshPeriodicRelativeZ(), MeshPeriodicDiffZ()
@*/
double MeshPeriodicZ(Mesh mesh, double z)
{
  PetscFunctionBegin;
  if (mesh->isPeriodicDim[2] == PETSC_FALSE)
    PetscFunctionReturn(z);
  if (z - PETSC_MACHINE_EPSILON > mesh->endZ) {
    if (z - mesh->sizeZ > mesh->endZ) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one\n", z);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(z - mesh->sizeZ);
  } else if (z + PETSC_MACHINE_EPSILON < mesh->startZ) {
    if (z + mesh->sizeZ < mesh->startZ) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one\n", z);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(z + mesh->sizeZ);
  } else {
    PetscFunctionReturn(z);
  }
}


#undef  __FUNC__
#define __FUNC__ "MeshPeriodicRelativeX"
/*@
  MeshPeriodicRelativeX - Returns the value modulo the period of the mesh, relative
  to a given coordinate.


  Not collective


  Input Parameters:
+ mesh - The mesh
. x    - The original coordinate
- xO   - The origin defining the period


  Output Parameter:
. ret  - The normalized coordinate


  Level: intermediate


  Note:
  This function is normally used to have a geometric object reside in a single
  period of the mesh. For instance, a triangle that needs to be drawn.


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicRelativeY(), MeshPeriodicRelativeZ(), MeshPeriodicX(), MeshPeriodicDiffX()
@*/
double MeshPeriodicRelativeX(Mesh mesh, double x, double xO)
{
  PetscFunctionBegin;
  if (mesh->isPeriodicDim[0] == PETSC_FALSE)
    PetscFunctionReturn(x);
  if (x - PETSC_MACHINE_EPSILON > xO + 0.5*mesh->sizeX) {
    if (x - mesh->sizeX > xO + mesh->sizeX) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one about %g\n", x, xO);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(x - mesh->sizeX);
  } else if (x + PETSC_MACHINE_EPSILON < xO - 0.5*mesh->sizeX) {
    if (x + mesh->sizeX < xO - mesh->sizeX) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one about %g\n", x, xO);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(x + mesh->sizeX);
  } else {
    PetscFunctionReturn(x);
  }
}


#undef  __FUNC__
#define __FUNC__ "MeshPeriodicRelativeY"
/*@
  MeshPeriodicRelativeY - Returns the value modulo the period of the mesh, relative
  to a given coordinate.


  Not collective


  Input Parameters:
+ mesh - The mesh
. y    - The original coordinate
- yO   - The origin defining the period


  Output Parameter:
. ret  - The normalized coordinate


  Level: intermediate


  Note:
  This function is normally used to have a geometric object reside in a single
  period of the mesh. For instance, a triangle that needs to be drawn.


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicRelativeX(), MeshPeriodicRelativeZ(), MeshPeriodicY(), MeshPeriodicDiffY()
@*/
double MeshPeriodicRelativeY(Mesh mesh, double y, double yO)
{
  PetscFunctionBegin;
  if (mesh->isPeriodicDim[1] == PETSC_FALSE)
    PetscFunctionReturn(y);
  if (y - PETSC_MACHINE_EPSILON > yO + 0.5*mesh->sizeY) {
    if (y - mesh->sizeY > yO + mesh->sizeY) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one about %g\n", y, yO);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(y - mesh->sizeY);
  } else if (y + PETSC_MACHINE_EPSILON < yO - 0.5*mesh->sizeY) {
    if (y + mesh->sizeY < yO - mesh->sizeY) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one about %g\n", y, yO);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(y + mesh->sizeY);
  } else {
    PetscFunctionReturn(y);
  }
}


#undef  __FUNC__
#define __FUNC__ "MeshPeriodicRelativeZ"
/*@
  MeshPeriodicRelativeZ - Returns the value modulo the period of the mesh, relative
  to a given coordinate.


  Not collective


  Input Parameters:
+ mesh - The mesh
. z    - The original coordinate
- zO   - The origin defining the period


  Output Parameter:
. ret  - The normalized coordinate


  Level: intermediate


  Note:
  This function is normally used to have a geometric object reside in a single
  period of the mesh. For instance, a triangle that needs to be drawn.


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicRelativeX(), MeshPeriodicRelativeY(), MeshPeriodicZ(), MeshPeriodicDiffZ()
@*/
double MeshPeriodicRelativeZ(Mesh mesh, double z, double zO)
{
  PetscFunctionBegin;
  if (mesh->isPeriodicDim[2] == PETSC_FALSE)
    PetscFunctionReturn(z);
  if (z - PETSC_MACHINE_EPSILON > zO + 0.5*mesh->sizeZ) {
    if (z - mesh->sizeZ > zO + mesh->sizeZ) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one about %g\n", z, zO);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(z - mesh->sizeZ);
  } else if (z + PETSC_MACHINE_EPSILON < zO - 0.5*mesh->sizeZ) {
    if (z + mesh->sizeZ < zO - mesh->sizeZ) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Coordinate %g has winding number greater than one about %g\n", z, zO);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(z + mesh->sizeZ);
  } else {
    PetscFunctionReturn(z);
  }
}


#undef  __FUNC__
#define __FUNC__ "MeshPeriodicDiffX"
/*@
  MeshPeriodicDiffX - Takes the difference of two x-coordinates in the domain, and returns
  the value modulo the period of the mesh.


  Not collective


  Input Parameters:
+ mesh - The mesh
- diff - The original difference


  Output Parameter:
. ret  - The normalized difference


  Level: intermediate


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicDiffY(), MeshPeriodicDiffZ(), MeshPeriodicX(), MeshPeriodicRelativeX()
@*/
double MeshPeriodicDiffX(Mesh mesh, double diff)
{
  PetscFunctionBegin;
  if (mesh->isPeriodicDim[0] == PETSC_FALSE)
    PetscFunctionReturn(diff);
  if (diff - PETSC_MACHINE_EPSILON > 0.5*mesh->sizeX) {
    if (diff - mesh->sizeX > 0.5*mesh->sizeX) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Difference %g has winding number greater than one\n", diff);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(diff - mesh->sizeX);
  } else if (diff + PETSC_MACHINE_EPSILON < -0.5*mesh->sizeX) {
    if (diff + mesh->sizeX < -0.5*mesh->sizeX) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Difference %g has winding number greater than one\n", diff);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(diff + mesh->sizeX);
  } else {
    PetscFunctionReturn(diff);
  }
}


#undef  __FUNC__
#define __FUNC__ "MeshPeriodicDiffY"
/*@
  MeshPeriodicDiffY - Takes the difference of two y-coordinates in the domain, and returns
  the value modulo the period of the mesh.


  Not collective


  Input Parameters:
+ mesh - The mesh
- diff - The original difference


  Output Parameter:
. ret  - The normalized difference


  Level: intermediate


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicDiffX(), MeshPeriodicDiffZ(), MeshPeriodicY(), MeshPeriodicRelativeY()
@*/
double MeshPeriodicDiffY(Mesh mesh, double diff)
{
  PetscFunctionBegin;
  if (mesh->isPeriodicDim[1] == PETSC_FALSE)
    PetscFunctionReturn(diff);
  if (diff - PETSC_MACHINE_EPSILON > 0.5*mesh->sizeY) {
    if (diff - mesh->sizeY > 0.5*mesh->sizeY) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Difference %g has winding number greater than one\n", diff);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(diff - mesh->sizeY);
  } else if (diff + PETSC_MACHINE_EPSILON < -0.5*mesh->sizeY) {
    if (diff + mesh->sizeY < -0.5*mesh->sizeY) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Difference %g has winding number greater than one\n", diff);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(diff + mesh->sizeY);
  } else {
    PetscFunctionReturn(diff);
  }
}


#undef  __FUNC__
#define __FUNC__ "MeshPeriodicDiffZ"
/*@
  MeshPeriodicDiffZ - Takes the difference of two z-coordinates in the domain, and returns
  the value modulo the period of the mesh.


  Not collective


  Input Parameters:
+ mesh - The mesh
- diff - The original difference


  Output Parameter:
. ret  - The normalized difference


  Level: intermediate


.keywords: mesh, coordinates, periodic
.seealso: MeshPeriodicDiffX(), MeshPeriodicDiffY(), MeshPeriodicZ(), MeshPeriodicRelativeZ()
@*/
double MeshPeriodicDiffZ(Mesh mesh, double diff)
{
  PetscFunctionBegin;
  if (mesh->isPeriodicDim[2] == PETSC_FALSE)
    PetscFunctionReturn(diff);
  if (diff - PETSC_MACHINE_EPSILON > 0.5*mesh->sizeZ) {
    if (diff - mesh->sizeZ > 0.5*mesh->sizeZ) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Difference %g has winding number greater than one\n", diff);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(diff - mesh->sizeZ);
  } else if (diff + PETSC_MACHINE_EPSILON < -0.5*mesh->sizeZ) {
    if (diff + mesh->sizeZ < -0.5*mesh->sizeZ) {
      PetscPrintf(PETSC_COMM_SELF, "ERROR: Difference %g has winding number greater than one\n", diff);
      PetscFunctionReturn(0.0);
    }
    PetscFunctionReturn(diff + mesh->sizeZ);
  } else {
    PetscFunctionReturn(diff);
  }
}


#endif /* MESH_PERIODIC_OPTIMIZED */


#undef  __FUNC__
#define __FUNC__ "MeshDrawLine"
/*@
  MeshDrawLine - This function draws a line, taking into account the
  periodicity of the mesh.


  Collective on Mesh


  Input Parameters:
+ mesh  - The mesh
. draw  - The Draw context
. xA,yA - The coordinates of an endpoint
. xB,yB - The coordinates of the other endpoint
- color - The line color


  Level: intermediate


.keywords mesh, draw
.seealso MeshDrawTriangle()
@*/
int MeshDrawLine(Mesh mesh, Draw draw, double xA, double yA, double xB, double yB, int color)
{
  int ierr;


  PetscFunctionBegin;
  if (mesh->isPeriodic == PETSC_TRUE) {
    ierr = DrawLine(draw, xA, yA, MeshPeriodicRelativeX(mesh, xB, xA), MeshPeriodicRelativeY(mesh, yB, yA), color);
    CHKERRQ(ierr);
  } else {
    ierr = DrawLine(draw, xA, yA, xB, yB, color);                                                        CHKERRQ(ierr);
  }
  PetscFunctionReturn(ierr);
}


#undef  __FUNC__
#define __FUNC__ "MeshDrawTriangle"
/*@
  MeshDrawLine - This function draws a triangle, taking into account the
  periodicity of the mesh.


  Collective on Mesh


  Input Parameters:
+ mesh  - The mesh
. draw  - The Draw context
. xA,yA - The coordinates of the first corner
. xB,yB - The coordinates of the second corner
. xC,yC - The coordinates of the third corner
. colorA, colorB, colorC - The colors of the corners


  Level: intermediate


.keywords mesh, draw
.seealso MeshDrawLine()
@*/
int MeshDrawTriangle(Mesh mesh, Draw draw, double xA, double yA, double xB, double yB, double xC, double yC, int colorA,
                     int colorB, int colorC)
{
  int ierr;


  PetscFunctionBegin;
  if (mesh->isPeriodic == PETSC_TRUE) {
    ierr = DrawTriangle(draw, xA, yA, MeshPeriodicRelativeX(mesh, xB, xA), MeshPeriodicRelativeY(mesh, yB, yA),
                        MeshPeriodicRelativeX(mesh, xC, xA), MeshPeriodicRelativeY(mesh, yC, yA), colorA, colorB, colorC);
    CHKERRQ(ierr);
  } else {
    ierr = DrawTriangle(draw, xA, yA, xB, yB, xC, yC, colorA, colorB, colorC);                           CHKERRQ(ierr);
  }
  PetscFunctionReturn(ierr);
}
