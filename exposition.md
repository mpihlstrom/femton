*[The LaTeX support in GitHub markdown is a great initiative but it's not working properly, for me. It's mainly the inline stuff which is buggy. I have tried to adapt, at some expense of coherence and legibility, unfortunately.]*

# Edge semantic triangulation: a technical exposition [draft]

This text primarily serves as a memorandum for myself, a documentation of the mathematical formulas used in this project and the outlining motivation for them. As such, the ambition has not been an exahustive description of the algorithmic content.

One of the main concerns and challenges of this project has been robustness. Specifically, much effort was put into treating concepts which are easily expressed with real numbers but which do not hold up in a software settting if implemented simply and naively with floating point representation. One major theme therefore has been to expand and transform real number expressions into a semi-algorithmic form with conditionals and (big) integers only. During development the slightest error in these formulas would typically result in a totally dysfunctional system. Because of this, not least, it seemed prudent to at some point and in some fashion expose parts of the more important but perhaps not so obvious trails leading up to the final implementation, before they are forgotten. Somewhat ironically, the code itself remains the definite exposition, albeit arcane, as any errors will have likely manifested in crashes and been corrected along the way. Though I'd like to imagine the algorithm is correct in a formal sense, this has far from been proved. Hopefully the reasoning presented here can be used as support for at least some notion of conviction that this is indeed the case.

## The algorithm 

### Outline 

For all triangles, calculate the t' (if any) when the triangle is collinear and calculate what edge at this t' needs to be flipped. Sort the triangles in a list by t' in ascending order. Pop and flip the first triangle. Since the topology of the flipped triangle pair is changed, the t' values for these triangles need to be re-calculated and the sorting updated. Continue popping and flipping until the list is empty.

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
    recursively remove all neighboring triangles that are also have three overlapping vertices
    stitch the gap
    ...
  else if exactly two vertices are overlapping:
    remove the triangle pair given by the overlapping vertices
    stitch the gap
  else
    flip the intersected edge
    recalculate t' and intersected edge for the triangle pair and push to H
```

## Planar violation calculations

*[Todo: write about superposing vertices!]*

Let a, b and c be the vertices of a triangle $ \triangle abc$ and let the position of each vertex be parameterized by $t$ as follows.

$$p = p_t = p(t) = p_0(1-t) + p_1t = p_0 + t(p_1 - p_0)$$

We want to determine the value(s) of t when $\triangle abc$ intersects itself, that is, when the edge vectors are collinear. We also want to know which edge is intersected by an opposing vertex for such values of t. We can determine that by finding the longest edge.

### The collinear triangle

We recall that two vectors are collinear iff the determinant of the 2 by 2 matrix they form is 0. Let's introduce a short-hand notation, the binary operation $\wedge$.

$$
\vec{u} \wedge \vec{v} := 
     \begin{vmatrix}
       u_x & v_x \\
       u_y & v_y \\
     \end{vmatrix}
= u_x v_y - u_y v_x
$$

From the expression $\vec{ab} \wedge \vec{ac}$, let us now factor out the powers of $t$ in standard polynomial form and dub the constant factors $A$, $B$ and $C$.

$$
\vec{ab} \wedge \vec{ac} =
$$

$$
\vec{ab}_x\vec{ac}_y - \vec{ab}_y\vec{ac}_x =
$$

$$
(b_x-a_x)(c_y-a_y) - (b_y-a_y)(c_x-a_x) = 
$$

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

$$
t = -\frac{B}{2A} \pm \sqrt{\left(\frac{B}{2A}\right)^2 - \frac{C}{A}}
$$

For convenience we express a single root t' by parameterizing the +/- sign with s in {-1,0,1}. We also introduce the short-hand constants $P = \frac{B}{2A} $ and $Q = \frac{C}{A}$.

$$
t' = -P + s \sqrt{P^2 - Q}
$$

For the ensuing inequality calculus it will be useful to borrow a notation from programming. The "spaceship operator", sometimes written <=> but we will simply write $\diamond$, is a binary operation such that

$$
a \diamond b = 
\begin{cases}
   -1, & \text{if } a \lt b \\
    1, & \text{if } a \gt b \\
    0, & \text{otherwise}
\end{cases}
$$

and we will abbreviate expressions of the form $x \diamond 0$ as $s_x$.
<p>
We are interested in comparing t' for two triangles. Let these be $t'_f$ and $t'_s$. We want to calculate $t'_f \diamond t'_s$. Because of computational constraints we cannot allow square roots and fractions in our expression. We can avoid these by expanding the expression and split it up in cases in the following (rather long-winding) way. Beginning with the general case when
 
$$A_f \neq 0 \land A_s \neq 0 $$

then
 
$$
t'_f \diamond t'_s = 
$$

$$
-P_f + s_f \sqrt{P_f^2 - Q_f}  \diamond -P_s + s_s \sqrt{P_s^2 - Q_s} = 
$$

$$
\underbrace{ P_s - P_f }_L  \diamond \underbrace{  s_s \sqrt{P_s^2 - Q_s} - s_f \sqrt{P_f^2 - Q_f}  }_R = 
$$

$$
\begin{bmatrix}
s_L = L \diamond 0 = P_s-P_f \diamond 0 = \frac{B_s}{2A_s} - \frac{B_f}{2A_f} \diamond 0 = (A_fA_s \diamond 0) (B_sA_f - B_fA_s) \diamond 0 \\
s_R = R \diamond 0 = s_s \sqrt{P_s^2 - Q_s} - s_f \sqrt{P_f^2 - Q_f} \diamond 0 = s_s(B_s^2A_f^2 - 4C_sA_sA_f^2) - s_f(B_f^2A_s^2 - 4C_f A_fA_s^2) \diamond 0
\end{bmatrix}
$$

$$
\begin{array}{l}
\text{if} & s_Ls_R \le 0 & \text{then} & s_L  - s_R \\
\text{if} & s_L = s_R & \text{then}
\end{array}
$$

$$
s_L \left(P_s - P_f \right)^2 \diamond s_R \left( s_s \sqrt{P_s^2 - Q_s} - s_f \sqrt{P_f^2 - Q_f} \right)^2 =
$$

$$
s_L \left(P_s^2 + P_f^2 - 2P_sP_f \right) \diamond s_R \left(P_s^2 - Q_s + P_f^2 - Q_f  - 2 s_s s_f \sqrt{P_s^2 - Q_s} \sqrt{P_f^2 - Q_f} \right) =
$$

$$
\underbrace{s_L \left(- 2P_sP_f + Q_s + Q_f\right)}_{L'} \diamond \underbrace{-2 s_R s_s s_f \sqrt{P_s^2 - Q_s} \sqrt{P_f^2 - Q_f}}_{R'}  =
$$
  
$$
\begin{bmatrix}
s_{L'} = L' \diamond 0 = s_L \left(- 2P_sP_f + Q_s + Q_f\right) \diamond 0 = s_L (A_sA_f \diamond 0)(-B_fB_s + 2C_fA_s + 2C_sA_s) \diamond 0 \\
s_{R'} = R' \diamond 0 = -s_R s_s s_f (P_s^2 - Q_s) (P_f^2 - Q_f) \diamond 0 =  -s_R s_s s_f (B_s^2 - 4 A_sC_s)(B_f^2 - 4 A_fC_f) \diamond 0
\end{bmatrix}
$$

$$
\begin{array}{l}
\text{if} & s_{L'} s_{R'} \le 0 & \text{then} & s_{L'}  - s_{R'} \\
\text{if} & s_{L'} = s_{R'} & \text{then}
\end{array}
$$
  
$$
s_{L'} {\left( s_L \left(- 2P_sP_f + Q_s + Q_f\right) \right)}^2 \diamond s_{R'} {\left( -2 s_R s_s s_f \sqrt{P_s^2 - Q_s} \sqrt{P_f^2 - Q_f} \right)}^2 =
$$

$$
s_{L'} \left((P_sP_f)^2 + (Q_s + Q_f)^2  - 4 (P_sP_f + Q_s + Q_f) \right) \diamond 4 s_{R'} (P_s^2 - Q_s) (P_f^2 - Q_f)  =
$$
  
$$
s_{L'} \left((P_sP_f)^2 + (Q_s + Q_f)^2  - 4 (P_sP_f + Q_s + Q_f) - 4 (P_s^2 - Q_s) (P_f^2 - Q_f) \right) \diamond 0  =
$$
  
$$
s_{L'} \left( C_f^2 A_s^2 + C_s^2 A_f^2 - 2C_fC_sA_fA_s - B_fB_sC_fA_s - B_fB_sC_sA_f + B_f^2C_sA_s + B_s^2C_fA_f \right) \diamond 0
$$

Next we cover the special case when 

$$A_f = 0 \land B_f \neq 0 \land A_s \neq 0$$
 
where

$$
f_f(t) = 0 \iff tB_f + C_f = 0 \iff t'_f = -\frac{C_f}{B_f}
$$

so that
  
$$
t'_f \diamond t'_s = 
$$

$$
-\frac{C_f}{B_f} \diamond  -P_s + s_s \sqrt{P_s^2 - Q_s} =
$$

$$
\underbrace{\frac{B_s}{2A_s} -\frac{C_f}{B_f}}_L = \underbrace{s_s \sqrt{\left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s}}}_R
$$

$$
\begin{bmatrix}
s_{L} = L \diamond 0 = (A_s \diamond 0)(B_f \diamond 0)(B_fB_s - 2C_fA_s) \diamond 0 \\
s_{R} = R \diamond 0 = s_s \sqrt{\left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s}} \diamond 0 = s_s \left( \left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s} \right) \diamond 0 = s_s(B_s^2 - 4C_sA_s) \diamond 0
\end{bmatrix}
$$

$$
\begin{array}{l}
\text{if} & s_Ls_R \le 0 & \text{then} & s_L  - s_R \\
\text{if} & s_L = s_R & \text{then}
\end{array}
$$

$$
s_L\left( \frac{B_s}{2A_s} -\frac{C_f}{B_f} \right)^2 \diamond s_R \left( s_s \sqrt{\left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s}} \right)^2 = 
$$

$$
s_L\left( \left( \frac{B_s}{2A_s} \right)^2 + \left( \frac{C_f}{B_f} \right)^2 - \frac{B_sC_f}{A_sB_f} \right) \diamond s_R \left( \left( \frac{B_s}{2A_s} \right)^2  - \frac{C_s}{A_s} \right) = 
$$

$$
s_L\left( \frac{C_f^2}{B_f^2} - \frac{B_sC_f}{A_sB_f} + \frac{C_s}{A_s} \right) \diamond 0 = 
$$

$$
(A_s \diamond 0 )s_L\left( C_f^2A_s - B_fB_sC_f + B_fB_sC_s \right) \diamond 0 
$$
  
Imaginary values of t' will not be of interest and are skipped if the following formula holds true.
 
$$(A = 0 \land B = 0) \lor (B^2 - 4CA \ge 0)$$

### The intersected edge

To determine the longest edge in a triangle it is sufficient to compare the square of their vector magnitudes. The dot product of a vector with itself can be refactored for t in the same way as the previously introduced determinant operator. Note that $\vec{u}_0 \cdot \vec{u}_1 = \vec{u}_1 \cdot \vec{u}_0$ and that the matrix is compacted.
  
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
 A' \\
 B' \\
 C' \\
\end{bmatrix}
$$
  
We want to determine the longest edge for some t' when the edges are collinear. As before, we cannot allow square roots and fractions, so the expressions have to be expanded. Without loss of generality, we derive the formulas for the difference between the squared magnitudes of vectors ab and bc. Beginning with the general case when
 
 $$A \neq 0$$
 
 we get the following formulas.

$$
g_{\vec{ab}}(t') \diamond g_{\vec{bc}}(t') =
$$
  
$$  
\begin{bmatrix}t'^2 & t' & 1 \end{bmatrix}
\begin{bmatrix}
 A' \\
 B' \\
 C' \\
\end{bmatrix} \diamond
\begin{bmatrix}t'^2 & t' & 1 \end{bmatrix}
\begin{bmatrix}
 A \\
 B' \\
 C' \\
\end{bmatrix} =
$$
  
$$
 A_\Delta \left( -P+s\sqrt{P^2-Q} \right)^2 + B_\Delta \left( -P+s\sqrt{P^2-Q} \right) + C_\Delta  \diamond 0 =
$$

$$
 \underbrace{A_\Delta (2P^2-Q) - B_\Delta P + C_\Delta}_L  \diamond \underbrace{(2A_\delta P - B_\delta) s  \overbrace{\sqrt{P^2-Q}}^{R'} }_R =
 $$
 
 $$
\begin{bmatrix}
s_{L} = L \diamond 0 = A_\Delta (2P^2-Q) - B_\Delta P + C_\Delta \diamond 0 = A_\Delta (B^2 - 2AC) - ABB_\Delta + 2A^2C_\Delta \diamond 0 \\
s_{R'} = R' \diamond 0 = \sqrt{P^2-Q} \diamond 0 = P^2 - Q \diamond 0 = B^2 - 4AC \diamond 0 \\
s_{R} = R \diamond 0 = (2A_\delta P - B_\delta) s \sqrt{P^2-Q} \diamond 0 = -(A \diamond 0)s_{R'}s(AB_\Delta - A_\Delta B) \diamond 0
\end{bmatrix}
 $$
 
 $$
 \begin{array}{l}
 \text{if} & s_Ls_R \le 0 & \text{then} & s_L  - s_R \\
 \text{if} & s_L = s_R & \text{then}
 \end{array}
 $$
 
 $$
 s_L (A_\Delta (2P^2-Q) - B_\Delta P + C_\Delta)^2  \diamond s_R \left((2A_\Delta P - B_\Delta) s \sqrt{P^2-Q}\right)^2  =
 $$
 
 $$
s_L \left(\frac{A_\Delta B^2}{2A^2} - \frac{A_\Delta C}{A} - \frac{B_\Delta}{2A} + C_\Delta \right)^2 \diamond s_R \left(\frac{B^2}{4A^2} - \frac{C}{A} \right) \left( B_\Delta - \frac{A_\Delta B}{A} \right)^2 =
$$

$$
s_L \left( \left( 2 A_\Delta B^2 - 4A_\Delta A C - 2B_\Delta AB + 4A^2 C_\Delta \right)^2  - (B^2 - 4AC)(2AB_\Delta - 2A_\Delta B)^2 \right) \diamond 0
$$

The special case when 

$$A = 0 \land B \neq 0$$
 
reduces to a shorter formula, as follows.

$$
g_{\vec{ab}}(t') \diamond g_{\vec{bc}}(t') =
$$
  
$$
A_\Delta \left(-\frac{C}{B} \right)^2 + B_\Delta \left(-\frac{C}{B} \right) + C_\Delta \diamond 0 =
$$
  
$$
A_\Delta C^2 -B_\Delta BC + C_\Delta B^2 \diamond 0
$$

Finally, we are also interested in determining degenerate cases when the collinear triangle has one or more overlapping vertices, that is, when some edge vector magnitude is 0. This can be checked by simply evaluating the expression $g_{\vec{ab}}(t') \diamond \vec{0}$.



