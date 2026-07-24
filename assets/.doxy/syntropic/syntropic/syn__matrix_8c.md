

# File syn\_matrix.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_matrix.c**](syn__matrix_8c.md)

[Go to the source code of this file](syn__matrix_8c_source.md)

_Fixed-point Q16.16 matrix operations implementation._ [More...](#detailed-description)

* `#include "syn_matrix.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_matrix\_add**](#function-syn_matrix_add) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* a, const [**SYN\_Matrix**](structSYN__Matrix.md) \* b, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_Element-wise addition: out = A + B._  |
|  void | [**syn\_matrix\_copy**](#function-syn_matrix_copy) ([**SYN\_Matrix**](structSYN__Matrix.md) \* dst, const [**SYN\_Matrix**](structSYN__Matrix.md) \* src) <br>_Copy matrix contents._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_matrix\_det**](#function-syn_matrix_det) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* m) <br>_Determinant of a square matrix._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_eigen\_sym2**](#function-syn_matrix_eigen_sym2) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* A, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) evals, [**SYN\_Matrix**](structSYN__Matrix.md) \* E) <br>_Compute eigenvalues and eigenvectors of a symmetric 2×2 matrix A._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_eigen\_sym3**](#function-syn_matrix_eigen_sym3) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* A, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) evals, [**SYN\_Matrix**](structSYN__Matrix.md) \* E) <br>_Compute eigenvalues and eigenvectors of a symmetric 3×3 matrix A via Jacobi rotations._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_get\_block**](#function-syn_matrix_get_block) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* src, uint8\_t r0, uint8\_t c0, [**SYN\_Matrix**](structSYN__Matrix.md) \* dst) <br>_Extract a submatrix block from src into dst._  |
|  void | [**syn\_matrix\_identity**](#function-syn_matrix_identity) ([**SYN\_Matrix**](structSYN__Matrix.md) \* m) <br>_Set matrix to identity._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_inv**](#function-syn_matrix_inv) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* m, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_Matrix inverse: out = M⁻¹._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_least\_squares**](#function-syn_matrix_least_squares) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* A, const [**SYN\_Matrix**](structSYN__Matrix.md) \* b, [**SYN\_Matrix**](structSYN__Matrix.md) \* x) <br>_Solve overdetermined system A · x ≈ b via Normal Equations (Least Squares)._  |
|  void | [**syn\_matrix\_mul**](#function-syn_matrix_mul) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* a, const [**SYN\_Matrix**](structSYN__Matrix.md) \* b, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_Matrix multiply: out = A × B._  |
|  void | [**syn\_matrix\_mul\_vec**](#function-syn_matrix_mul_vec) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* m, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v\_in, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v\_out, uint8\_t n\_in) <br>_Matrix–vector multiply: out = M × v._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_outer\_product**](#function-syn_matrix_outer_product) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* u, uint8\_t rows, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, uint8\_t cols, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_Vector outer product: out = u · vᵀ (rows(u) × cols(v))._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_qr**](#function-syn_matrix_qr) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* A, [**SYN\_Matrix**](structSYN__Matrix.md) \* Q, [**SYN\_Matrix**](structSYN__Matrix.md) \* R) <br>_QR Decomposition: A = Q · R via Modified Gram-Schmidt process._  |
|  void | [**syn\_matrix\_rotate\_2d**](#function-syn_matrix_rotate_2d) ([**SYN\_Matrix**](structSYN__Matrix.md) \* out, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) angle) <br>_Set 3×3 matrix to a 2D rotation._  |
|  void | [**syn\_matrix\_rotate\_x**](#function-syn_matrix_rotate_x) ([**SYN\_Matrix**](structSYN__Matrix.md) \* out, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) angle) <br>_Set 4×4 matrix to a rotation around the X axis._  |
|  void | [**syn\_matrix\_rotate\_y**](#function-syn_matrix_rotate_y) ([**SYN\_Matrix**](structSYN__Matrix.md) \* out, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) angle) <br>_Set 4×4 matrix to a rotation around the Y axis._  |
|  void | [**syn\_matrix\_rotate\_z**](#function-syn_matrix_rotate_z) ([**SYN\_Matrix**](structSYN__Matrix.md) \* out, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) angle) <br>_Set 4×4 matrix to a rotation around the Z axis._  |
|  void | [**syn\_matrix\_scale**](#function-syn_matrix_scale) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* a, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) scalar, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_Scalar multiply: out = A × scalar._  |
|  void | [**syn\_matrix\_scale\_2d**](#function-syn_matrix_scale_2d) ([**SYN\_Matrix**](structSYN__Matrix.md) \* out, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) sx, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) sy) <br>_Set 3×3 matrix to a 2D scale._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_set\_block**](#function-syn_matrix_set_block) ([**SYN\_Matrix**](structSYN__Matrix.md) \* dst, uint8\_t r0, uint8\_t c0, const [**SYN\_Matrix**](structSYN__Matrix.md) \* src) <br>_Embed a submatrix block src into dst at position (r0, c0)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_solve\_cholesky**](#function-syn_matrix_solve_cholesky) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* A, const [**SYN\_Matrix**](structSYN__Matrix.md) \* b, [**SYN\_Matrix**](structSYN__Matrix.md) \* x) <br>_Solve symmetric positive-definite system A · x = b via Cholesky._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_matrix\_solve\_lu**](#function-syn_matrix_solve_lu) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* A, const [**SYN\_Matrix**](structSYN__Matrix.md) \* b, [**SYN\_Matrix**](structSYN__Matrix.md) \* x) <br>_Solve general linear system A · x = b via LU decomposition._  |
|  void | [**syn\_matrix\_sub**](#function-syn_matrix_sub) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* a, const [**SYN\_Matrix**](structSYN__Matrix.md) \* b, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_Element-wise subtraction: out = A − B._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_matrix\_trace**](#function-syn_matrix_trace) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* m) <br>_Trace (sum of diagonal elements)._  |
|  void | [**syn\_matrix\_translate\_2d**](#function-syn_matrix_translate_2d) ([**SYN\_Matrix**](structSYN__Matrix.md) \* out, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) tx, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) ty) <br>_Set 3×3 matrix to a 2D translation._  |
|  void | [**syn\_matrix\_translate\_3d**](#function-syn_matrix_translate_3d) ([**SYN\_Matrix**](structSYN__Matrix.md) \* out, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) tx, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) ty, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) tz) <br>_Set 4×4 matrix to a 3D translation._  |
|  void | [**syn\_matrix\_transpose**](#function-syn_matrix_transpose) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* a, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_Transpose: out = Aᵀ._  |
|  void | [**syn\_matrix\_zero**](#function-syn_matrix_zero) ([**SYN\_Matrix**](structSYN__Matrix.md) \* m) <br>_Set all elements to zero._  |
|  void | [**syn\_vec3\_cross**](#function-syn_vec3_cross) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* a, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* b, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out) <br>_Cross product of two 3-element Q16 vectors._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_vec\_dot**](#function-syn_vec_dot) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* a, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* b, uint8\_t n) <br>_Dot product of two n-element Q16 vectors._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_vec\_norm**](#function-syn_vec_norm) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, uint8\_t n) <br>_Magnitude (Euclidean norm) of an n-element Q16 vector._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_vec\_normalize**](#function-syn_vec_normalize) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* v, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out, uint8\_t n) <br>_Normalize a vector to unit length._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**det\_2x2**](#function-det_2x2) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* d) <br>_2×2 determinant: ad - bc._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**det\_3x3**](#function-det_3x3) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* d) <br>_3×3 determinant via cofactor expansion along row 0._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**det\_4x4**](#function-det_4x4) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* d) <br>_4×4 determinant via cofactor expansion along row 0._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**inv\_1x1**](#function-inv_1x1) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* m, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_1×1 inverse via scalar division: 1/a._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**inv\_2x2**](#function-inv_2x2) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* m, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_2×2 inverse via adjugate/determinant._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**inv\_3x3**](#function-inv_3x3) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* m, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_3×3 inverse via adjugate/determinant._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**inv\_4x4**](#function-inv_4x4) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* m, [**SYN\_Matrix**](structSYN__Matrix.md) \* out) <br>_4×4 inverse via Gauss-Jordan elimination with partial pivoting._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SOLVER\_MAX\_N**](syn__matrix_8c.md#define-syn_solver_max_n)  `16`<br> |

## Detailed Description


All operations use caller-owned storage. No heap allocation. int64\_t accumulators preserve full Q16 precision in multiply chains. 


    
## Public Functions Documentation




### function syn\_matrix\_add 

_Element-wise addition: out = A + B._ 
```C++
void syn_matrix_add (
    const SYN_Matrix * a,
    const SYN_Matrix * b,
    SYN_Matrix * out
) 
```





**Parameters:**


* `a` First operand. 
* `b` Second operand (same dimensions as a). 
* `out` Result (same dimensions as a). 




        

<hr>



### function syn\_matrix\_copy 

_Copy matrix contents._ 
```C++
void syn_matrix_copy (
    SYN_Matrix * dst,
    const SYN_Matrix * src
) 
```





**Parameters:**


* `dst` Destination (must match src dimensions). 
* `src` Source matrix. 




        

<hr>



### function syn\_matrix\_det 

_Determinant of a square matrix._ 
```C++
q16_t syn_matrix_det (
    const SYN_Matrix * m
) 
```



Supports 2×2, 3×3, and 4×4. Uses cofactor expansion for 2×2/3×3 and LU decomposition for 4×4.




**Parameters:**


* `m` Square matrix. 



**Returns:**

Determinant in Q16. 





        

<hr>



### function syn\_matrix\_eigen\_sym2 

_Compute eigenvalues and eigenvectors of a symmetric 2×2 matrix A._ 
```C++
SYN_Status syn_matrix_eigen_sym2 (
    const SYN_Matrix * A,
    q16_t evals,
    SYN_Matrix * E
) 
```





**Parameters:**


* `A` Symmetric 2×2 matrix. 
* `evals` Output array of 2 eigenvalues (sorted descending). 
* `E` Output 2×2 matrix whose columns are the normalized eigenvectors. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_matrix\_eigen\_sym3 

_Compute eigenvalues and eigenvectors of a symmetric 3×3 matrix A via Jacobi rotations._ 
```C++
SYN_Status syn_matrix_eigen_sym3 (
    const SYN_Matrix * A,
    q16_t evals,
    SYN_Matrix * E
) 
```





**Parameters:**


* `A` Symmetric 3×3 matrix. 
* `evals` Output array of 3 eigenvalues (sorted descending). 
* `E` Output 3×3 matrix whose columns are the normalized eigenvectors. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_matrix\_get\_block 

_Extract a submatrix block from src into dst._ 
```C++
SYN_Status syn_matrix_get_block (
    const SYN_Matrix * src,
    uint8_t r0,
    uint8_t c0,
    SYN_Matrix * dst
) 
```





**Parameters:**


* `src` Source matrix (M×N). 
* `r0` Starting row index in src. 
* `c0` Starting column index in src. 
* `dst` Destination matrix (R×C). Must fit within src bounds from (r0, c0). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on out-of-bounds. 





        

<hr>



### function syn\_matrix\_identity 

_Set matrix to identity._ 
```C++
void syn_matrix_identity (
    SYN_Matrix * m
) 
```





**Parameters:**


* `m` Square matrix. 




        

<hr>



### function syn\_matrix\_inv 

_Matrix inverse: out = M⁻¹._ 
```C++
SYN_Status syn_matrix_inv (
    const SYN_Matrix * m,
    SYN_Matrix * out
) 
```



Supports 2×2, 3×3, and 4×4 square matrices. Returns SYN\_ERROR if the matrix is singular (determinant ≈ 0).




**Parameters:**


* `m` Input square matrix. 
* `out` Output inverse (same dimensions). Must not alias m. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if singular. 





        

<hr>



### function syn\_matrix\_least\_squares 

_Solve overdetermined system A · x ≈ b via Normal Equations (Least Squares)._ 
```C++
SYN_Status syn_matrix_least_squares (
    const SYN_Matrix * A,
    const SYN_Matrix * b,
    SYN_Matrix * x
) 
```



Minimizes \|\|A·x − b\|\|₂ for an M×N system (M ≥ N). Solves (Aᵀ · A) · x = Aᵀ · b using Cholesky or LU decomposition.




**Parameters:**


* `A` Overdetermined system matrix (M×N, M ≥ N). 
* `b` Right-hand side measurement vector (M×1). 
* `x` Output solution parameter vector (N×1). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if AᵀA is singular. 





        

<hr>



### function syn\_matrix\_mul 

_Matrix multiply: out = A × B._ 
```C++
void syn_matrix_mul (
    const SYN_Matrix * a,
    const SYN_Matrix * b,
    SYN_Matrix * out
) 
```



Requires: A.cols == B.rows, out is A.rows × B.cols. Uses int64\_t accumulator for full Q16 precision.




**Parameters:**


* `a` Left operand (R₁ × C₁). 
* `b` Right operand (C₁ × C₂). 
* `out` Result (R₁ × C₂). Must not alias a or b. 




        

<hr>



### function syn\_matrix\_mul\_vec 

_Matrix–vector multiply: out = M × v._ 
```C++
void syn_matrix_mul_vec (
    const SYN_Matrix * m,
    const q16_t * v_in,
    q16_t * v_out,
    uint8_t n_in
) 
```





**Parameters:**


* `m` Square or rectangular matrix (R × C). 
* `v_in` Input vector (C elements). 
* `v_out` Output vector (R elements). Must not alias v\_in. 
* `n_in` Length of input vector (must equal m-&gt;cols). 




        

<hr>



### function syn\_matrix\_outer\_product 

_Vector outer product: out = u · vᵀ (rows(u) × cols(v))._ 
```C++
SYN_Status syn_matrix_outer_product (
    const q16_t * u,
    uint8_t rows,
    const q16_t * v,
    uint8_t cols,
    SYN_Matrix * out
) 
```





**Parameters:**


* `u` Column vector elements (R elements). 
* `rows` Number of rows (R). 
* `v` Row vector elements (C elements). 
* `cols` Number of columns (C). 
* `out` Output matrix (R×C). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL or invalid dimensions. 





        

<hr>



### function syn\_matrix\_qr 

_QR Decomposition: A = Q · R via Modified Gram-Schmidt process._ 
```C++
SYN_Status syn_matrix_qr (
    const SYN_Matrix * A,
    SYN_Matrix * Q,
    SYN_Matrix * R
) 
```



Decomposes an M×N matrix A (M ≥ N) into orthogonal M×N matrix Q (QᵀQ = I) and upper-triangular N×N matrix R.




**Parameters:**


* `A` Input matrix (M×N, M ≥ N). 
* `Q` Output orthogonal matrix (M×N). 
* `R` Output upper-triangular matrix (N×N). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if A is linearly dependent / singular. 





        

<hr>



### function syn\_matrix\_rotate\_2d 

_Set 3×3 matrix to a 2D rotation._ 
```C++
void syn_matrix_rotate_2d (
    SYN_Matrix * out,
    q16_t angle
) 
```





**Parameters:**


* `out` Output 3×3 matrix. 
* `angle` Rotation angle in Q16 radians (counter-clockwise). 




        

<hr>



### function syn\_matrix\_rotate\_x 

_Set 4×4 matrix to a rotation around the X axis._ 
```C++
void syn_matrix_rotate_x (
    SYN_Matrix * out,
    q16_t angle
) 
```





**Parameters:**


* `out` Output 4×4 matrix. 
* `angle` Rotation angle in Q16 radians. 




        

<hr>



### function syn\_matrix\_rotate\_y 

_Set 4×4 matrix to a rotation around the Y axis._ 
```C++
void syn_matrix_rotate_y (
    SYN_Matrix * out,
    q16_t angle
) 
```





**Parameters:**


* `out` Output 4×4 matrix. 
* `angle` Rotation angle in Q16 radians. 




        

<hr>



### function syn\_matrix\_rotate\_z 

_Set 4×4 matrix to a rotation around the Z axis._ 
```C++
void syn_matrix_rotate_z (
    SYN_Matrix * out,
    q16_t angle
) 
```





**Parameters:**


* `out` Output 4×4 matrix. 
* `angle` Rotation angle in Q16 radians. 




        

<hr>



### function syn\_matrix\_scale 

_Scalar multiply: out = A × scalar._ 
```C++
void syn_matrix_scale (
    const SYN_Matrix * a,
    q16_t scalar,
    SYN_Matrix * out
) 
```





**Parameters:**


* `a` Input matrix. 
* `scalar` Q16 scalar multiplier. 
* `out` Result (same dimensions as a). 




        

<hr>



### function syn\_matrix\_scale\_2d 

_Set 3×3 matrix to a 2D scale._ 
```C++
void syn_matrix_scale_2d (
    SYN_Matrix * out,
    q16_t sx,
    q16_t sy
) 
```





**Parameters:**


* `out` Output 3×3 matrix. 
* `sx` X scale factor in Q16. 
* `sy` Y scale factor in Q16. 




        

<hr>



### function syn\_matrix\_set\_block 

_Embed a submatrix block src into dst at position (r0, c0)._ 
```C++
SYN_Status syn_matrix_set_block (
    SYN_Matrix * dst,
    uint8_t r0,
    uint8_t c0,
    const SYN_Matrix * src
) 
```





**Parameters:**


* `dst` Destination matrix (M×N). 
* `r0` Starting row index in dst. 
* `c0` Starting column index in dst. 
* `src` Source submatrix block (R×C). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on out-of-bounds. 





        

<hr>



### function syn\_matrix\_solve\_cholesky 

_Solve symmetric positive-definite system A · x = b via Cholesky._ 
```C++
SYN_Status syn_matrix_solve_cholesky (
    const SYN_Matrix * A,
    const SYN_Matrix * b,
    SYN_Matrix * x
) 
```



Decomposes A into L · Lᵀ where L is lower-triangular, then solves L · y = b and Lᵀ · x = y. Faster and more numerically stable than LU for symmetric positive-definite matrices (e.g. covariance or normal eqns).




**Parameters:**


* `A` Symmetric positive-definite system matrix (N×N). 
* `b` Right-hand side vector (N×1). 
* `x` Output solution vector (N×1). May alias b. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if A is not positive-definite. 





        

<hr>



### function syn\_matrix\_solve\_lu 

_Solve general linear system A · x = b via LU decomposition._ 
```C++
SYN_Status syn_matrix_solve_lu (
    const SYN_Matrix * A,
    const SYN_Matrix * b,
    SYN_Matrix * x
) 
```



Uses Doolittle LU decomposition with partial pivoting (PA = LU). A must be a square N×N non-singular matrix. b and x must be N×1 vectors.




**Parameters:**


* `A` Square system matrix (N×N). 
* `b` Right-hand side vector (N×1). 
* `x` Output solution vector (N×1). May alias b. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if A is singular or dimensions invalid. 





        

<hr>



### function syn\_matrix\_sub 

_Element-wise subtraction: out = A − B._ 
```C++
void syn_matrix_sub (
    const SYN_Matrix * a,
    const SYN_Matrix * b,
    SYN_Matrix * out
) 
```





**Parameters:**


* `a` Minuend. 
* `b` Subtrahend (same dimensions as a). 
* `out` Result (same dimensions as a). 




        

<hr>



### function syn\_matrix\_trace 

_Trace (sum of diagonal elements)._ 
```C++
q16_t syn_matrix_trace (
    const SYN_Matrix * m
) 
```





**Parameters:**


* `m` Square matrix. 



**Returns:**

Trace in Q16. 





        

<hr>



### function syn\_matrix\_translate\_2d 

_Set 3×3 matrix to a 2D translation._ 
```C++
void syn_matrix_translate_2d (
    SYN_Matrix * out,
    q16_t tx,
    q16_t ty
) 
```





**Parameters:**


* `out` Output 3×3 matrix. 
* `tx` X translation in Q16. 
* `ty` Y translation in Q16. 




        

<hr>



### function syn\_matrix\_translate\_3d 

_Set 4×4 matrix to a 3D translation._ 
```C++
void syn_matrix_translate_3d (
    SYN_Matrix * out,
    q16_t tx,
    q16_t ty,
    q16_t tz
) 
```





**Parameters:**


* `out` Output 4×4 matrix. 
* `tx` X translation in Q16. 
* `ty` Y translation in Q16. 
* `tz` Z translation in Q16. 




        

<hr>



### function syn\_matrix\_transpose 

_Transpose: out = Aᵀ._ 
```C++
void syn_matrix_transpose (
    const SYN_Matrix * a,
    SYN_Matrix * out
) 
```





**Parameters:**


* `a` Input (R × C). 
* `out` Output (C × R). Must not alias a. 




        

<hr>



### function syn\_matrix\_zero 

_Set all elements to zero._ 
```C++
void syn_matrix_zero (
    SYN_Matrix * m
) 
```





**Parameters:**


* `m` Matrix to clear. 




        

<hr>



### function syn\_vec3\_cross 

_Cross product of two 3-element Q16 vectors._ 
```C++
void syn_vec3_cross (
    const q16_t * a,
    const q16_t * b,
    q16_t * out
) 
```





**Parameters:**


* `a` First vector (3 elements). 
* `b` Second vector (3 elements). 
* `out` Output vector (3 elements). Must not alias a or b. 




        

<hr>



### function syn\_vec\_dot 

_Dot product of two n-element Q16 vectors._ 
```C++
q16_t syn_vec_dot (
    const q16_t * a,
    const q16_t * b,
    uint8_t n
) 
```





**Parameters:**


* `a` First vector. 
* `b` Second vector. 
* `n` Number of elements. 



**Returns:**

Dot product in Q16. 





        

<hr>



### function syn\_vec\_norm 

_Magnitude (Euclidean norm) of an n-element Q16 vector._ 
```C++
q16_t syn_vec_norm (
    const q16_t * v,
    uint8_t n
) 
```





**Parameters:**


* `v` Vector. 
* `n` Number of elements. 



**Returns:**

\|v\| in Q16. 





        

<hr>



### function syn\_vec\_normalize 

_Normalize a vector to unit length._ 
```C++
SYN_Status syn_vec_normalize (
    const q16_t * v,
    q16_t * out,
    uint8_t n
) 
```





**Parameters:**


* `v` Input vector (n elements). 
* `out` Output unit vector (n elements). May alias v. 
* `n` Number of elements. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if zero-length vector. 





        

<hr>
## Public Static Functions Documentation




### function det\_2x2 

_2×2 determinant: ad - bc._ 
```C++
static q16_t det_2x2 (
    const q16_t * d
) 
```





**Parameters:**


* `d` Flat 4-element array [a, b, c, d] (row-major). 



**Returns:**

Determinant in Q16. 





        

<hr>



### function det\_3x3 

_3×3 determinant via cofactor expansion along row 0._ 
```C++
static q16_t det_3x3 (
    const q16_t * d
) 
```





**Parameters:**


* `d` Flat 9-element array (row-major). 



**Returns:**

Determinant in Q16. 





        

<hr>



### function det\_4x4 

_4×4 determinant via cofactor expansion along row 0._ 
```C++
static q16_t det_4x4 (
    const q16_t * d
) 
```





**Parameters:**


* `d` Flat 16-element array (row-major). 



**Returns:**

Determinant in Q16. 





        

<hr>



### function inv\_1x1 

_1×1 inverse via scalar division: 1/a._ 
```C++
static SYN_Status inv_1x1 (
    const SYN_Matrix * m,
    SYN_Matrix * out
) 
```





**Parameters:**


* `m` Input 1×1 matrix. 
* `out` Output 1×1 inverse. 



**Returns:**

SYN\_OK or SYN\_ERROR if zero. 





        

<hr>



### function inv\_2x2 

_2×2 inverse via adjugate/determinant._ 
```C++
static SYN_Status inv_2x2 (
    const SYN_Matrix * m,
    SYN_Matrix * out
) 
```





**Parameters:**


* `m` Input 2×2 matrix. 
* `out` Output 2×2 inverse. 



**Returns:**

SYN\_OK or SYN\_ERROR if singular. 





        

<hr>



### function inv\_3x3 

_3×3 inverse via adjugate/determinant._ 
```C++
static SYN_Status inv_3x3 (
    const SYN_Matrix * m,
    SYN_Matrix * out
) 
```





**Parameters:**


* `m` Input 3×3 matrix. 
* `out` Output 3×3 inverse. 



**Returns:**

SYN\_OK or SYN\_ERROR if singular. 





        

<hr>



### function inv\_4x4 

_4×4 inverse via Gauss-Jordan elimination with partial pivoting._ 
```C++
static SYN_Status inv_4x4 (
    const SYN_Matrix * m,
    SYN_Matrix * out
) 
```





**Parameters:**


* `m` Input 4×4 matrix. 
* `out` Output 4×4 inverse. 



**Returns:**

SYN\_OK or SYN\_ERROR if singular. 





        

<hr>
## Macro Definition Documentation





### define SYN\_SOLVER\_MAX\_N 

```C++
#define SYN_SOLVER_MAX_N `16`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_matrix.c`

