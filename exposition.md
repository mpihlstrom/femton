*This text primarily serves as a memo for myself, a documentation of the mathematical formulas used in this project and the outlining motivation for them. As such, the ambition has not been an exahustive description of the algorithmic content.*

# Edge-semantic triangulation: a technical exposition

In the triangulation algorithm of this project, floating point errors were avoided by expanding real number equations into a computer-friendly form involving conditionals and (big) integers only. This document aims to explain these passages found in the source code. Though the provided content does not constitute a formal proof of correctness, hopefully it can support some such notion.

## The algorithm 

### Outline 

For all triangles, calculate the $t'$ (if any) for when the triangle is collinear and calculate what edge at this $t'$ needs to be flipped. Sort the triangles in a list by $t'$ in ascending order. Pop and flip the first triangle. Since the topology of the flipped triangle pair is changed, the $t'$ values for these triangles need to be re-calculated and the sorting updated. Continue popping and flipping until the list is empty.

Overlapping vertices is a degenerate case of a collinear triangles. Instead of being flipped, such triangles will be removed and the mesh will to be stitched up in the appropriate way.

### Pseudo code

```
//init stage
set q = 0 
create heap H (sorted by ascending values of t')
for all triangles T:
  calculate the smallest collinear t' for T such that t' > q and  t' <= 1
  if t' exists:
    determine the intersected edge(s) at t'
    push T to H sorted on t'

// main loop
while H is not empty
  extract the top T in H (smallest t')
  if all three vertices of T are overlapping:
    recursively remove all neighboring triangles that also have three overlapping vertices
  else if exactly two vertices are overlapping:
    remove the triangle pair given by the overlapping vertices
  else
    flip the intersected edge
    recalculate t' and intersected edge for the triangle pair and push to H
```

## Self-intersecting triangles

Let $a$, $b$ and $c$ be the vertices of a triangle $\triangle abc$ and let the position of each vertex be parameterized by $t$ as follows.

$$p(t) := p_0 + t(p_1 - p_0)$$

We want to determine the value(s) of $t$ when $\triangle abc$ intersects itself, that is, when the edge vectors are collinear. Importantly, because it is only the order of timepoints $t$ of different self-intersecting triangles which will ever be needed, it will not be necessary to calculate the value of $t$ explicitly.

We recall that two vectors are collinear iff the determinant of the $2 \times 2$ matrix they form is 0. Let's introduce a short-hand notation, the binary operation $\wedge$.

$$
\vec{u} \wedge \vec{v} := 
     \begin{vmatrix}
       u_x & v_x \\
       u_y & v_y \\
     \end{vmatrix}
= u_x v_y - u_y v_x
$$

From the expression $\vec{ab} \wedge \vec{ac}$, let us now factor out the powers of $t$ into standard polynomial form and introduce the constants $A$, $B$ and $C$.

$$\vec{ab} \wedge \vec{ac} =$$

$$\vec{ab}_x\vec{ac}_y - \vec{ab}_y\vec{ac}_x =$$

$$(b_x-a_x)(c_y-a_y) - (b_y-a_y)(c_x-a_x) = $$

$$
\begin{array}{l}
[{b_0}_x + t({b_1}_x - {b_0}_x) - {a_0}_x + t({a_1}_x - {a_0}_x)]
[{c_0}_y + t({c_1}_y - {c_0}_y) - {a_0}_y + t({a_1}_y - {a_0}_y)] - \\
[{b_0}_y + t({b_1}_y - {b_0}_y) - {a_0}_y + t({a_1}_y - {a_0}_y)]
[{c_0}_x + t({c_1}_x - {c_0}_x) - {a_0}_x + t({a_1}_x - {a_0}_x)] =
\end{array}
$$

$$
\underbrace{(\vec{ab_0} \wedge \vec{ac_0} - \vec{ab_0} \wedge \vec{ac_1} -  \vec{ab_1} \wedge \vec{ac_0} + \vec{ab_1} \wedge \vec{ac_1})}_A t^2 +
\underbrace{(-2\vec{ab_0} \wedge \vec{ac_0} + \vec{ab_0} \wedge \vec{ac_1} +  \vec{ab_1} \wedge \vec{ac_0})}_B t + \underbrace{\vec{ab_0} \wedge \vec{ac_0}}_C
$$ 

Let us define the polynomial function $f(t)$ and write the above expression in matrix form. 

$$
f(t) := 
\begin{bmatrix}
t^2 & t & 1 \\
\end{bmatrix}
\begin{bmatrix}
 1 & -1 & -1 & 1 \\
 -2 & 1 & 1 & 0 \\
 1 & 0 & 0 & 0 \\
\end{bmatrix}
\begin{bmatrix}
\vec{ab}_0 \wedge \vec{ac}_0 \\
\vec{ab}_0 \wedge \vec{ac}_1 \\
\vec{ab}_1 \wedge \vec{ac}_0 \\
\vec{ab}_1 \wedge \vec{ac}_1 \\
\end{bmatrix} = 
\begin{bmatrix}
t^2 & t & 1 \\
\end{bmatrix}
\begin{bmatrix}
A \\
B \\
C \\
\end{bmatrix}
$$

The triangle $\triangle abc$ will self-intersect when $f(t) = 0$. We recall the closed formula for solving $t$.

$$t = -\frac{B}{2A} \pm \sqrt{\left(\frac{B}{2A}\right)^2 - \frac{C}{A}}$$

For convenience, we express a single root $t'$ by parameterizing the $\pm$ sign with $s \in$ {-1,0,1}. We also introduce the constants $P = \frac{B}{2A}$ and $Q = \frac{C}{A}$.

$$t' = -P + s \sqrt{P^2 - Q}$$

For the ensuing inequality calculus it will be useful to borrow a notation from programming. We introduce the "spaceship operator", sometimes written <=> but we will simply write $\diamond$, defined as follows.

$$
a \diamond b := 
\begin{cases}
   -1, & \text{if } a \lt b \\
    1, & \text{if } a \gt b \\
    0, & \text{otherwise}
\end{cases}
$$

We also define

$$s_\mathcal{X} := \mathcal{X} \diamond 0$$

as an abbreviated form for the sign of an expression $\mathcal{X}$.

We are interested in comparing $t'$ for two triangles. Let these be $t'_f$ and $t'_s$. We want to calculate $t'_f \diamond t'_s$. Because of computational constraints, we cannot allow square roots and fractions in our expression. We will avoid these by expanding the expression so that it contains only integers and conditionals, split up in cases depending on the degree of the polynomials.

### The quadratic case

Beginning with the general case when both polynomials are quadratic, when
 
$$A_f \neq 0 \land A_s \neq 0 $$

then
 
$$t'_f \diamond t'_s = $$

$$-P_f + s_f \sqrt{P_f^2 - Q_f}  \diamond -P_s + s_s \sqrt{P_s^2 - Q_s} = $$

$$\underbrace{ P_s - P_f }_L  \diamond \underbrace{  s_s \sqrt{P_s^2 - Q_s} - s_f \sqrt{P_f^2 - Q_f}  }_R = $$

$$
\begin{bmatrix}
s_L = P_s-P_f \diamond 0 = \frac{B_s}{2A_s} - \frac{B_f}{2A_f} \diamond 0 = s_{A_f}s_{A_s} (B_sA_f - B_fA_s) \diamond 0 \\
s_R = s_s \sqrt{P_s^2 - Q_s} - s_f \sqrt{P_f^2 - Q_f} \diamond 0 = s_s(B_s^2A_f^2 - 4C_sA_sA_f^2) - s_f(B_f^2A_s^2 - 4C_f A_fA_s^2) \diamond 0
\end{bmatrix}
$$

$$
\begin{array}{l}
\text{if} & s_Ls_R \le 0 & \text{then} & s_L  - s_R \\
\text{if} & s_L = s_R & \text{then}
\end{array}
$$

$$s_L \left(P_s - P_f \right)^2 \diamond s_R \left( s_s \sqrt{P_s^2 - Q_s} - s_f \sqrt{P_f^2 - Q_f} \right)^2 =$$

$$s_L \left(P_s^2 + P_f^2 - 2P_sP_f \right) \diamond s_R \left(P_s^2 - Q_s + P_f^2 - Q_f  - 2 s_s s_f \sqrt{P_s^2 - Q_s} \sqrt{P_f^2 - Q_f} \right) =$$

$$\underbrace{s_L \left(- 2P_sP_f + Q_s + Q_f\right)}_{L'}  \diamond  $$

$$\underbrace{-2 s_R s_s s_f \sqrt{P_s^2 - Q_s} \sqrt{P_f^2 - Q_f}}_{R'} =$$

  
$$
\begin{bmatrix}
s_{L'} = s_L \left(- 2P_sP_f + Q_s + Q_f\right) \diamond 0 = s_L s_{A_s}s_{A_f}(-B_fB_s + 2C_fA_s + 2C_sA_s) \diamond 0 \\
s_{R'} = -s_R s_s s_f (P_s^2 - Q_s) (P_f^2 - Q_f) \diamond 0 =  -s_R s_s s_f (B_s^2 - 4 A_sC_s)(B_f^2 - 4 A_fC_f) \diamond 0
\end{bmatrix}
$$

$$
\begin{array}{l}
\text{if} & s_{L'} s_{R'} \le 0 & \text{then} & s_{L'}  - s_{R'} \\
\text{if} & s_{L'} = s_{R'} & \text{then}
\end{array}
$$
  
$$s_{L'} {\left( s_L \left(- 2P_sP_f + Q_s + Q_f\right) \right)}^2 \diamond s_{R'} {\left( -2 s_R s_s s_f \sqrt{P_s^2 - Q_s} \sqrt{P_f^2 - Q_f} \right)}^2 =$$

$$s_{L'} \left((P_sP_f)^2 + (Q_s + Q_f)^2  - 4 (P_sP_f + Q_s + Q_f) \right) \diamond 4 s_{R'} (P_s^2 - Q_s) (P_f^2 - Q_f)  =$$
  
$$s_{L'} \left((P_sP_f)^2 + (Q_s + Q_f)^2  - 4 (P_sP_f + Q_s + Q_f) - 4 (P_s^2 - Q_s) (P_f^2 - Q_f) \right) \diamond 0  =$$
  
$$s_{L'} \left( C_f^2 A_s^2 + C_s^2 A_f^2 - 2C_fC_sA_fA_s - B_fB_sC_fA_s - B_fB_sC_sA_f + B_f^2C_sA_s + B_s^2C_fA_f \right) \diamond 0$$

### The quadratic and linear case

Next, for the special case when 

$$A_f = 0 \land B_f \neq 0 \land A_s \neq 0$$
 
where one polynomial is quadratic and the other is linear, where

$$ f_f(t) = 0 \iff tB_f + C_f = 0 \iff t'_f = -\frac{C_f}{B_f} $$

then
  
$$ t'_f \diamond t'_s =  $$

$$ -\frac{C_f}{B_f} \diamond  -P_s + s_s \sqrt{P_s^2 - Q_s} = $$

$$ \underbrace{\frac{B_s}{2A_s} -\frac{C_f}{B_f}}_L = \underbrace{s_s \sqrt{\left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s}}}_R $$

$$
\begin{bmatrix}
s_{L} = s_{A_s}s_{B_f}(B_fB_s - 2C_fA_s) \diamond 0 \\
s_{R} = s_s \sqrt{\left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s}} \diamond 0 = s_s \left( \left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s} \right) \diamond 0 = s_s(B_s^2 - 4C_sA_s) \diamond 0
\end{bmatrix}
$$

$$
\begin{array}{l}
\text{if} & s_Ls_R \le 0 & \text{then} & s_L  - s_R \\
\text{if} & s_L = s_R & \text{then}
\end{array}
$$

$$ s_L\left( \frac{B_s}{2A_s} -\frac{C_f}{B_f} \right)^2 \diamond s_R \left( s_s \sqrt{\left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s}} \right)^2 =  $$

$$ s_L\left( \left( \frac{B_s}{2A_s} \right)^2 + \left( \frac{C_f}{B_f} \right)^2 - \frac{B_sC_f}{A_sB_f} \right) \diamond s_R \left( \left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s} \right) = $$

$$ s_L\left( \frac{C_f^2}{B_f^2} - \frac{B_sC_f}{A_sB_f} + \frac{C_s}{A_s} \right) \diamond 0 = $$

$$ s_{A_s}s_L\left( C_f^2A_s - B_fB_sC_f + B_fB_sC_s \right) \diamond 0  $$
  
### The linear only case

For the special case when

$$A_f = 0 \land B_f \neq 0 \land A_s = 0 \land B_s \neq 0$$
 
where both polynomials are linear, then trivially

$$ t'_f \diamond t'_s = $$

$$ -\frac{C_f}{B_f} \diamond -\frac{C_s}{B_s} = $$

$$ s_{B_f} s_{B_s} (C_{s} B_{f} - C_{f} B_{b}) \diamond 0 $$
  
### The imaginary case

Finally, for cases where the roots are imaginary, further calculations can be skipped by checking if the following condition is true.
 
$$(A = 0 \land B = 0) \lor (B^2 - 4CA \ge 0)$$

## The intersected edge

For a triangle self-intersecting at t, we now want to determine out specifically which edge is intersected by an opposing vertex at that timepoint, which amounts to determining which edge is the longest. This is the edge that will be flipped.

To determine the longest edge in a triangle, it is sufficient to compare the square of their vector lengths. The dot product of a vector with itself can be refactored for $t$ in the same way as the previously introduced determinant operator. Note that $\vec{u}_0 \cdot \vec{u}_1 = \vec{u}_1 \cdot \vec{u}_0$ so that the matrix can be compacted. The constants $D$, $E$ and $F$ are introduced.
  
$$ g(t) := \vec{u} \cdot \vec{u} = 
\begin{bmatrix}
  t^2 & t & 1 \\
\end{bmatrix}
\begin{bmatrix}
  1 & -2 & 1 \\
  -2 & 2 & 0 \\
  1 & 0 & 0 \\
\end{bmatrix}  
\begin{bmatrix}
  \vec{u}_0 \cdot \vec{u}_0 \\
  \vec{u}_0 \cdot \vec{u}_1 \\
  \vec{u}_1 \cdot \vec{u}_1 \\
\end{bmatrix} =
\begin{bmatrix}
  t^2 & t & 1 \\
\end{bmatrix}
\begin{bmatrix}
 D \\
 E \\
 F \\
\end{bmatrix}
$$
  
We want to determine the longest edge for some $t'$ when the edges are collinear. As before, we cannot allow square roots and fractions, so the expressions have to be expanded. Without loss of generality, we derive the formulas for the comparison between the squared magnitudes of vectors $\vec{ab}$ and $\vec{bc}$. The constants $D$, $E$, and $F$ are subscripted with their corresponding vector, e.g. $D_\vec{ab}$ and $D_\vec{bc}$, and the difference between those constants are shortened with delta notation, e.g. $D_\Delta := D_\vec{ab}$ - $D_\vec{bc}$.

### The quadratic case

Beginning with the general case when
 
 $$A \neq 0$$
 
 then

$$g_{\vec{ab}}(t') \diamond g_{\vec{bc}}(t') =$$

$$g_{\vec{ab}}(t') - g_{\vec{bc}}(t') \diamond 0 =$$
  
$$  
\begin{bmatrix}t'^2 & t' & 1 \end{bmatrix}
\begin{bmatrix}
 D_\vec{ab} \\
 E_\vec{ab} \\
 F_\vec{ab} \\
\end{bmatrix} -
\begin{bmatrix}t'^2 & t' & 1 \end{bmatrix}
\begin{bmatrix}
 D_\vec{bc} \\
 E_\vec{bc} \\
 F_\vec{bc} \\
\end{bmatrix} \diamond 0 =
$$
  
$$  
\begin{bmatrix}t'^2 & t' & 1 \end{bmatrix}
\begin{bmatrix}
 D_\vec{ab} - D_\vec{bc} \\
 E_\vec{ab} - E_\vec{bc} \\
 F_\vec{ab} - F_\vec{ab} \\
\end{bmatrix} \diamond 0 =
$$
  
$$  
\begin{bmatrix}t'^2 & t' & 1 \end{bmatrix}
\begin{bmatrix}
 D_\Delta \\
 E_\Delta \\
 F_\Delta \\
\end{bmatrix} \diamond 0 =
$$
  
$$D_\Delta \left( -P+s\sqrt{P^2-Q} \right)^2 + E_\Delta \left( -P+s\sqrt{P^2-Q} \right) + F_\Delta  \diamond 0 =$$

$$\underbrace{D_\Delta (2P^2-Q) - E_\Delta P + F_\Delta}_L \diamond$$

$$ \underbrace{(2D_\Delta P - E_\Delta) s  \overbrace{\sqrt{P^2-Q}}^{R'} }_R = $$
 
 $$
\begin{bmatrix}
s_{L} = D_\Delta (2P^2-Q) - E_\Delta P + F_\Delta \diamond 0 = D_\Delta (B^2 - 2AC) - ABE_\Delta + 2A^2F_\Delta \diamond 0 \\
s_{R'} = \sqrt{P^2-Q} \diamond 0 = P^2 - Q \diamond 0 = B^2 - 4AC \diamond 0 \\
s_{R} = (2D_\Delta P - E_\Delta) s \sqrt{P^2-Q} \diamond 0 = -s_A s(AE_\Delta - D_\Delta B)s_{R'} \diamond 0
\end{bmatrix}
 $$
 
 $$
 \begin{array}{l}
 \text{if} & s_Ls_R \le 0 & \text{then} & s_L  - s_R \\
 \text{if} & s_L = s_R & \text{then}
 \end{array}
 $$
 
 $$s_L (D_\Delta (2P^2-Q) - E_\Delta P + F_\Delta)^2  \diamond s_R \left((2D_\Delta P - E_\Delta) s \sqrt{P^2-Q}\right)^2  =$$
 $$s_L \left(\frac{D_\Delta B^2}{2A^2} - \frac{D_\Delta C}{A} - \frac{E_\Delta}{2A} + F_\Delta \right)^2 \diamond s_R \left(\frac{B^2}{4A^2} - \frac{C}{A} \right) \left( B_\Delta - \frac{A_\Delta B}{A} \right)^2 =$$
$$s_L \left( \left( 2 D_\Delta B^2 - 4D_\Delta A C - 2E_\Delta AB + 4A^2 F_\Delta \right)^2  - (B^2 - 4AC)(2AE_\Delta - 2D_\Delta B)^2 \right) \diamond 0$$

### The linear case

The special linear case when 

$$A = 0 \land B \neq 0$$
 
reduces to a shorter formula, as follows.

$$g_{\vec{ab}}(t') \diamond g_{\vec{bc}}(t') =$$

$$D_\Delta \left(-\frac{C}{B} \right)^2 + E_\Delta \left(-\frac{C}{B} \right) + F_\Delta \diamond 0 =$$  

$$D_\Delta C^2 -E_\Delta BC + F_\Delta B^2 \diamond 0$$

## Overlapping vertices  
  
I can happen that vertices cross paths in such a way that they precisely overlap at some timepoint. When that happens, we will want to amend the mesh by removing triangles with overlapping vertices and stitch the adjacent triangles together. A triangle with overlapping vertices is a degenerate case of a collinear triangle where the length of one or more edges is zero. This amounts to determing whether
  
$$g(t') \diamond \vec{0} = 0$$
  
is true. We observe that the left-hand side amounts to
  
$$  
\begin{bmatrix}t'^2 & t' & 1 \end{bmatrix}
\begin{bmatrix}
 D_\vec{ab} - 0 \\
 E_\vec{ab} - 0 \\
 F_\vec{ab} - 0 \\
\end{bmatrix} \diamond 0 =
$$

and that we can make use of the calculus from the preceding section by realizing that simply $D_\Delta = D'$, $E_\Delta = E'$ and $F_\Delta = F'$.
