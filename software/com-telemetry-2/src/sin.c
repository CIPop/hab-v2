/*
 * sin.c - Trigonometric functions
 */

#include "main.h"
#include "dac.h"

// Cannot fit 16384 entries in memory, go with fewer
#define SIN_TABLE_ENTRIES 512
#define SIN_SCALAR (B_RAD / (4 * SIN_TABLE_ENTRIES))

// SIN[x] should be 65536 * sin(pi * x / 32768), except having 16,384 16bit sine entries
// is too much; therefore, SIN[x] = 65536 * sin(pi * x / 2048)
static const uint16_t SIN[SIN_TABLE_ENTRIES + 1] = {
	0, 201, 402, 603, 804, 1005, 1206, 1407, 1608, 1809, 2010, 2211, 2412, 2613, 2813, 3014,
	3215, 3416, 3617, 3817, 4018, 4219, 4419, 4620, 4821, 5021, 5221, 5422, 5622, 5823, 6023,
	6223, 6423, 6623, 6823, 7023, 7223, 7423, 7622, 7822, 8022, 8221, 8421, 8620, 8819, 9018,
	9218, 9417, 9615, 9814, 10013, 10212, 10410, 10609, 10807, 11005, 11203, 11402, 11599,
	11797, 11995, 12193, 12390, 12587, 12785, 12982, 13179, 13376, 13573, 13769, 13966, 14162,
	14358, 14554, 14750, 14946, 15142, 15337, 15533, 15728, 15923, 16118, 16313, 16508, 16702,
	16896, 17091, 17285, 17479, 17672, 17866, 18059, 18252, 18445, 18638, 18831, 19023, 19216,
	19408, 19600, 19791, 19983, 20174, 20366, 20557, 20747, 20938, 21128, 21319, 21509, 21699,
	21888, 22078, 22267, 22456, 22645, 22833, 23021, 23210, 23398, 23585, 23773, 23960, 24147,
	24334, 24520, 24707, 24893, 25079, 25264, 25450, 25635, 25820, 26004, 26189, 26373, 26557,
	26741, 26924, 27107, 27290, 27473, 27655, 27837, 28019, 28201, 28382, 28563, 28744, 28925,
	29105, 29285, 29465, 29644, 29823, 30002, 30181, 30359, 30537, 30715, 30892, 31070, 31247,
	31423, 31599, 31775, 31951, 32126, 32302, 32476, 32651, 32825, 32999, 33172, 33346, 33519,
	33691, 33864, 34035, 34207, 34378, 34549, 34720, 34891, 35061, 35230, 35400, 35569, 35737,
	35906, 36074, 36241, 36409, 36576, 36742, 36909, 37075, 37240, 37406, 37571, 37735, 37899,
	38063, 38227, 38390, 38553, 38715, 38877, 39039, 39200, 39361, 39521, 39682, 39841, 40001,
	40160, 40319, 40477, 40635, 40792, 40950, 41106, 41263, 41419, 41574, 41730, 41885, 42039,
	42193, 42347, 42500, 42653, 42805, 42957, 43109, 43260, 43411, 43561, 43711, 43861, 44010,
	44159, 44307, 44455, 44603, 44750, 44896, 45043, 45189, 45334, 45479, 45623, 45768, 45911,
	46055, 46197, 46340, 46482, 46623, 46764, 46905, 47045, 47185, 47324, 47463, 47601, 47739,
	47877, 48014, 48151, 48287, 48422, 48558, 48693, 48827, 48961, 49094, 49227, 49360, 49492,
	49623, 49754, 49885, 50015, 50145, 50274, 50403, 50531, 50659, 50786, 50913, 51039, 51165,
	51291, 51415, 51540, 51664, 51787, 51910, 52033, 52155, 52276, 52397, 52518, 52638, 52757,
	52876, 52995, 53113, 53230, 53347, 53464, 53580, 53695, 53810, 53925, 54039, 54152, 54265,
	54378, 54490, 54601, 54712, 54823, 54933, 55042, 55151, 55259, 55367, 55474, 55581, 55687,
	55793, 55898, 56003, 56107, 56211, 56314, 56416, 56518, 56620, 56721, 56821, 56921, 57021,
	57119, 57218, 57316, 57413, 57509, 57606, 57701, 57796, 57891, 57985, 58078, 58171, 58263,
	58355, 58446, 58537, 58627, 58717, 58806, 58894, 58982, 59069, 59156, 59242, 59328, 59413,
	59498, 59582, 59665, 59748, 59830, 59912, 59993, 60074, 60154, 60234, 60313, 60391, 60469,
	60546, 60623, 60699, 60774, 60849, 60924, 60997, 61071, 61143, 61215, 61287, 61358, 61428,
	61498, 61567, 61636, 61704, 61771, 61838, 61904, 61970, 62035, 62100, 62163, 62227, 62290,
	62352, 62413, 62474, 62535, 62595, 62654, 62713, 62771, 62828, 62885, 62941, 62997, 63052,
	63107, 63161, 63214, 63267, 63319, 63370, 63421, 63472, 63521, 63570, 63619, 63667, 63714,
	63761, 63807, 63853, 63898, 63942, 63986, 64029, 64072, 64114, 64155, 64196, 64236, 64275,
	64314, 64353, 64390, 64427, 64464, 64500, 64535, 64570, 64604, 64637, 64670, 64702, 64734,
	64765, 64795, 64825, 64854, 64883, 64911, 64938, 64965, 64991, 65017, 65042, 65066, 65090,
	65113, 65135, 65157, 65178, 65199, 65219, 65238, 65257, 65275, 65293, 65310, 65326, 65342,
	65357, 65371, 65385, 65399, 65411, 65423, 65435, 65445, 65456, 65465, 65474, 65482, 65490,
	65497, 65504, 65510, 65515, 65519, 65523, 65527, 65530, 65532, 65533, 65534, 65535
};

// Calculates the sine of the unsigned angle in brad (65536)
int32_t sinfp(uint32_t angle) {
	bool sgn = false;
	uint32_t li, remainder;
	int32_t ret;
#if 0
	angle %= B_RAD;
#else
	angle &= (B_RAD - 1);
#endif
	// sin(x|x>pi) = -sin(x-pi)
	if (angle > (B_RAD >> 1)) {
		sgn = true;
		angle -= (B_RAD >> 1);
	}
	// sin(x|x>pi/2) = sin(pi-x)
	if (angle > (B_RAD >> 2))
		angle = (B_RAD >> 1) - angle;
	// Linearly interpolate between SIN table entries; if the compiler misses this optimization
	// into a shift, I will personally...
	// angle is in 0 .. (B_RAD >> 2)
	li = angle / SIN_SCALAR;
	remainder = angle - (li * SIN_SCALAR);
	// remainder is from 0..SIN_SCALAR
	ret = (int32_t)SIN[li];
	ret += (((int32_t)SIN[li + 1] - ret) * remainder + (SIN_SCALAR >> 1)) / SIN_SCALAR;
	if (sgn) ret = -ret;
	return ret;
}
