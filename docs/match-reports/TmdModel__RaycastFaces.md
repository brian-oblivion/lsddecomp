# TmdModel__RaycastFaces -- MATCHED (486/486 words), round 82

> Renamed from `func_8001F8B8` on 2026-09-25 (tools/rename.py). Address 0x8001f8b8.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/code_fa50.c`. Fresh ground, no prior attempt.

- **What:** segment-vs-model hit test. It walks every primitive of the model with `TmdModel__NextPrimitive`. For each one it builds the plane from the first three vertices (`OuterProduct0` of two edges, /4096, `d = -(v0 . n)`) and intersects the segment `origin -> end` with it. The parameter is a 16.16 fixed-point quotient of two fraction structs. It rejects a parallel face (status 2), a negative t, and a hit beyond the segment length (`Square0`/`SquareRoot0` on both vectors). It then checks the hit against the face's bounding box grown by 24, and keeps the nearest hit: `*best` = distance, `*hitOut` = point, `*height` = point.y - box.min.y (when `height != NULL`). Returns 1 if any face was hit. Callers: `code_d294_b.c`, `code_d294_c.c` (their own local prototypes, not touched).
- **Result:** byte-exact; 486/486 words, whole-image SHA1 green. Build 12 on this function.
- **Builds / levers, measured:**
  1. first draft (all locals at function top, `org`/`dir` as separate Vec3, `ABS(x) = x < 0 ? -x : x`): 10/486, frame 0x1A0.
  2. **locals that only the loop body uses declared INSIDE the `while` body, and `org`+`dir` as one 12-byte struct**: frame right (0x160); `nverts`/`count` land at 0x58/0x5C because their slots are made when `&nverts` is expanded in the `while` condition, BEFORE the body block's locals. 13/486.
  3. **`ABS(x)` as `x < 0 ? ~x + 1 : x`**: 106/486. Retail negates with `nor; addiu 1`, and with `-x` GCC's CSE carried values across the resulting branches instead of reloading from the frame.
  4. inverted `if` for the quotient (`q != 0` first), separate `q`/`hi` locals, success branch first for `len >= dist`, a union so the box lives in the scratch VECTOR at 0xF0 (retail addresses it as `0xF0(sp)` directly, not through a pointer variable), `dist` as a memory object: 462/486, right length. Left: the remainder numerator and the final t shared one register (t0 vs a2).
  5. separate `frac` local for `ABS(n % d) << 16`: 466/486. The frame was 0x38 too big (spare VECTORs I had added to fill 0x104..0x13F).
  6. removing the spare VECTORs: frame 0x168. `s32 dist[1]` went into a register (an extra `s7` save, size changed).
  7. **`s32 dist[2]`** (8 bytes, 4-byte aligned, so BLKmode, so memory): 486/486.
- **Frame, measured:** the 0x38 bytes above the locals (0x104..0x13F in retail) are dead reload slots. The six box-field pointers have `sp+K` equivalences, and each of the loop's six compares plus the `nverts - 1` bound adds 8 bytes to `vars=` in cc1's `.frame` comment (delete them one at a time and it shrinks by 8 each time), with no access ever made to them. Do not fill that gap with locals.
- **Types:** new `Ray_fa50`, `Vec4_fa50` (LIBGTE `VECTOR`, local), `VecBox_fa50`, the `ABS_fa50` macro, local prototypes for `OuterProduct0`/`Square0`/`SquareRoot0` (Sony libgte, called and not written) and for `TmdModel__NextPrimitive` (defined after this function). All in this unit.

## Source

```c
typedef struct Vec3_fa50 { s16 x, y, z; } Vec3_fa50;
typedef struct SVec_fa50 { s16 x, y, z, pad; } SVec_fa50;
typedef struct Box_fa50 { Vec3_fa50 min; Vec3_fa50 max; } Box_fa50;
typedef struct Ray_fa50 { Vec3_fa50 org; Vec3_fa50 dir; } Ray_fa50;
typedef struct Vec4_fa50 { s32 vx, vy, vz, pad; } Vec4_fa50;
typedef union VecBox_fa50 { Vec4_fa50 v; Box_fa50 b; } VecBox_fa50;
#define ABS_fa50(x) ((x) < 0 ? ~(x) + 1 : (x))
extern void OuterProduct0(Vec4_fa50 *v0, Vec4_fa50 *v1, Vec4_fa50 *v2);
extern void Square0(Vec4_fa50 *v0, Vec4_fa50 *v1);
extern s32 SquareRoot0(s32 a);
/* TmdPrim_fa50 / TmdModel / TmdModel__NextPrimitive: see TmdModel__NextPrimitive.md */

s32 TmdModel__RaycastFaces(TmdModel *self, s32 *best, Vec3_fa50 *hitOut, s32 *height, Vec3_fa50 *origin, Vec3_fa50 *end) {
    Vec3_fa50 tri[4];
    Vec4_fa50 plane;
    Ray_fa50 ray;
    Vec3_fa50 hit;
    s32 nverts;
    u32 count;
    TmdPrim_fa50 *p;
    s32 found;

    count = 0;
    *best = 0x7FFFFFFF;
    ray.org.x = origin->x;
    ray.org.y = origin->y;
    ray.org.z = origin->z;
    ray.dir.x = end->x - origin->x;
    ray.dir.y = end->y - origin->y;
    ray.dir.z = end->z - origin->z;
    found = 0;
    while ((p = TmdModel__NextPrimitive(self, p, &nverts, tri, &count)) != NULL) {
        Vec4_fa50 v60;
        SVec_fa50 e1;
        SVec_fa50 e2;
        Vec4_fa50 v80;
        Vec4_fa50 v90;
        Vec4_fa50 vA0;
        Vec4_fa50 vB0;
        Vec4_fa50 vC0;
        Vec4_fa50 vD0;
        Vec4_fa50 vE0;
        VecBox_fa50 uF0;
        s32 dist[2];
        s32 frac;
        s32 q;
        s32 hi;
        s32 t;
        s16 *minx;
        s16 *miny;
        s16 *minz;
        s16 *maxx;
        s16 *maxy;
        s16 *maxz;
        Vec3_fa50 *v;
        s32 i;

        e1.x = tri[1].x - tri[0].x;
        e1.y = tri[1].y - tri[0].y;
        e1.z = tri[1].z - tri[0].z;
        e2.x = tri[2].x - tri[0].x;
        e2.y = tri[2].y - tri[0].y;
        e2.z = tri[2].z - tri[0].z;
        v90.vx = e1.x;
        v90.vy = e1.y;
        v90.vz = e1.z;
        vA0.vx = e2.x;
        vA0.vy = e2.y;
        vA0.vz = e2.z;
        OuterProduct0(&v90, &vA0, &v80);
        plane.vx = v80.vx;
        plane.vy = v80.vy;
        plane.vz = v80.vz;
        plane.vx /= 4096;
        plane.vy /= 4096;
        plane.vz /= 4096;
        plane.pad = -(tri[0].x * plane.vx + tri[0].y * plane.vy + tri[0].z * plane.vz);
        vC0.vx = ray.dir.x * plane.vx + ray.dir.y * plane.vy + ray.dir.z * plane.vz;
        if (ABS_fa50(vC0.vx) <= 0) {
            vC0.vy = 2;
        } else {
            vC0.vz = ray.org.x * plane.vx + ray.org.y * plane.vy + ray.org.z * plane.vz;
            vC0.vz += plane.pad;
            v80.vx = vC0.vx;
            v80.vy = 1;
            vB0.vx = -vC0.vz;
            vB0.vy = 1;
            vB0.vz = vB0.vx * v80.vy;
            vB0.pad = vB0.vy * v80.vx;
            if (ABS_fa50(vB0.pad) >= 0x1000) {
                vB0.vz /= 4096;
                vB0.pad /= 4096;
            }
            frac = ABS_fa50(vB0.vz % vB0.pad) << 16;
            q = vB0.vz / vB0.pad;
            if (q != 0) {
                hi = q << 16;
            } else {
                hi = (vB0.vz * vB0.pad) & 0x80000000;
            }
            t = hi | (frac / ABS_fa50(vB0.pad));
            if (t < 0) {
                vC0.vy = 0;
            } else {
                vD0.vx = ray.dir.x;
                vD0.vy = ray.dir.y;
                vD0.vz = ray.dir.z;
                vA0.vx = (ray.dir.x * t) >> 16;
                v90.vx = ray.org.x + vA0.vx;
                vA0.vx = v90.vx - ray.org.x;
                vA0.vy = (ray.dir.y * t) >> 16;
                v90.vy = ray.org.y + vA0.vy;
                vA0.vy = v90.vy - ray.org.y;
                vA0.vz = (ray.dir.z * t) >> 16;
                v90.vz = ray.org.z + vA0.vz;
                vA0.vz = v90.vz - ray.org.z;
                Square0(&vD0, &vE0);
                vC0.vx = vE0.vx + vE0.vy + vE0.vz;
                vC0.vx = SquareRoot0(vC0.vx);
                Square0(&vA0, &uF0.v);
                vC0.vz = uF0.v.vx + uF0.v.vy + uF0.v.vz;
                vC0.vz = SquareRoot0(vC0.vz);
                if (vC0.vx >= vC0.vz) {
                    dist[0] = vC0.vz;
                    vC0.vy = 1;
                    hit.x = v90.vx;
                    hit.y = v90.vy;
                    hit.z = v90.vz;
                } else {
                    vC0.vy = 0;
                }
            }
        }
        if (vC0.vy != 1) {
            continue;
        }
        minx = &uF0.b.min.x;
        miny = &uF0.b.min.y;
        minz = &uF0.b.min.z;
        maxx = &uF0.b.max.x;
        maxy = &uF0.b.max.y;
        maxz = &uF0.b.max.z;
        v = tri;
        uF0.b.min = *v;
        uF0.b.max = uF0.b.min;
        for (i = 0; i < nverts - 1; i++) {
            v++;
            if (v->x < *minx) *minx = v->x;
            if (v->y < *miny) *miny = v->y;
            if (v->z < *minz) *minz = v->z;
            if (*maxx < v->x) *maxx = v->x;
            if (*maxy < v->y) *maxy = v->y;
            if (*maxz < v->z) *maxz = v->z;
        }
        if (hit.x < uF0.b.min.x - 24 || hit.y < uF0.b.min.y - 24 || hit.z < uF0.b.min.z - 24 ||
            uF0.b.max.x + 24 < hit.x || uF0.b.max.y + 24 < hit.y || uF0.b.max.z + 24 < hit.z) {
            continue;
        }
        found = 1;
        if (dist[0] < *best) {
            *best = dist[0];
            *hitOut = hit;
            if (height != NULL) {
                *height = hit.y - uF0.b.min.y;
            }
        }
    }
    return found;
}
```

### Proposed learning

- `nor v, zero, x; addiu v, v, 1` in an absolute value is `~x + 1`, not `-x` (which gives `negu`). The spelling also changes CSE across the branch: with `-x`, values stayed in registers where retail reloads them from the frame. 106 words turned on this one macro.
- A scalar that retail keeps in a frame slot with no address-take, while an equivalent `s32` gets a callee-saved register, can be an 8-byte `s32 x[2]`. It is BLKmode under STRICT_ALIGNMENT (4-byte alignment < DImode's 8), so it lives in memory. `s32 x[1]` gets SImode and a register.
- Dead reload slots (pseudos with `sp+K` equivalences) enlarge the frame with no access to them. Measure them from cc1's `.frame ... vars=` comment by deleting uses, before padding a frame gap with invented locals.
- Locals whose frame slots sit AFTER address-taken scalars used in a `while` condition were declared inside the loop body.

## Naming

`TmdModel__RaycastFaces` -- tier A. Casts a segment (`origin`..`end`)
against every primitive of the model (via `TmdModel__NextPrimitive`),
rejects a parallel or out-of-range hit, checks the candidate hit against the
face's own bounding box (grown by 24), and keeps the nearest one:
`*best`/`*hitOut`/`*height`. Returns whether anything was hit. Mechanics
fully describe the function; the game-level purpose (what calls this with
what segment) is settled by its callers in `code_d294_b.c`/`code_d294_c.c`,
not touched by this pass.
