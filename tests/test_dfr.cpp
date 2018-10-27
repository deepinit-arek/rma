/*
 * test_dfr.cpp
 *
 *  Created on: May 10, 2018
 *      Author: dleo@cwi.nl
 */


#include <iostream>
#include <memory>

#define CATCH_CONFIG_MAIN
#include "third-party/catch/catch.hpp"

#include "pma/external/dfr/dfr.hpp"

using namespace pma::dfr;
using namespace std;

TEST_CASE("sanity"){
    PackedMemoryArray pma;

    REQUIRE(pma.size() == 0);
    pma.insert(5, 50);
    pma.insert(6, 60);
    pma.insert(1, 10);
    pma.insert(3, 30);
    pma.insert(2, 20);
    pma.insert(10, 100);
    pma.insert(4, 40);
    pma.insert(11, 110);
    REQUIRE(pma.size() == 8);

//    pma.dump();

    auto it = pma.iterator();
    int64_t prev = -1;
//    cout << "Iterator: \n";
    while(it->hasNext()){
        auto p = it->next();
//        cout << "<" << p.first << ", " << p.second << ">" << endl;
        REQUIRE(prev < p.first);
        prev = p.first;
    }

    for(int64_t i = 0; i <= 12; i++){
        auto res = pma.find(i);
        // Elements 1, 2, 3, 4, 5, 6, 10, 11 are present and should be found
        if((i >= 1 && i <= 6) || i == 10 || i == 11){
            REQUIRE(res == i * 10);
        } else {
            REQUIRE(res == -1);
        }
    }
}

TEST_CASE("find"){
    PackedMemoryArray pma;

    // random permutation of all multiples of 3 from 3 up to 300
    int64_t sample[] = {60, 21, 159, 102, 231, 267, 30, 87, 138, 90, 33, 63, 195, 48, 222,
            180, 177, 288, 147, 225, 171, 132, 66, 54, 282, 261, 258, 240, 174,
            246, 120, 150, 201, 252, 36, 24, 96, 84, 45, 99, 51, 15, 153, 186,
            189, 213, 297, 183, 144, 219, 198, 3, 105, 111, 117, 300, 294, 81,
            18, 93, 210, 228, 75, 129, 57, 135, 168, 207, 156, 42, 6, 270, 78,
            165, 114, 192, 27, 279, 276, 12, 237, 162, 234, 9, 249, 273, 243,
            255, 126, 72, 69, 291, 123, 285, 216, 204, 264, 39, 108, 141};
    constexpr int64_t sample_sz = sizeof(sample) / sizeof(sample[0]);

    REQUIRE(pma.size() == 0);
    for(int64_t i = 0; i < sample_sz; i++){
        pma.insert(sample[i], sample[i] * 10);
    }
    REQUIRE(pma.size() == sample_sz);

    for(int i = 1; i <= 302; i++){
        auto v = pma.find(i);
        if(i % 3 == 0){
            REQUIRE(v == i * 10);
        } else {
            REQUIRE(v == -1);
        }
    }
}

TEST_CASE("range_queries"){
    PackedMemoryArray pma;

    int64_t sample[] = {68, 8, 71, 89, 66, 6, 18, 26, 51, 58, 62, 48, 94, 41, 2, 80, 72, 50,
            10, 92, 37, 21, 35, 17, 79, 77, 42, 39, 11, 86, 20, 75, 49, 40, 31,
            12, 98, 69, 3, 73, 45, 84, 57, 23, 93, 44, 54, 13, 46, 97, 78, 32, 5,
            14, 34, 96, 33, 56, 25, 9, 87, 83, 1, 22, 100, 70, 91, 99, 65, 60,
            85, 74, 64, 81, 82, 63, 55, 59, 38, 30, 90, 61, 19, 27, 7, 36, 53,
            15, 24, 95, 67, 52, 43, 16, 88, 4, 28, 29, 47, 76};
    constexpr int64_t sample_sz = sizeof(sample) / sizeof(sample[0]);

    for(int i = 0; i < sample_sz; i++){
        pma.insert(sample[i], sample[i] * 10);
    }

//    pma.dump();

    for(int i = 0; i <= sample_sz +1; i++){
        for(int j = i; j <= sample_sz +1; j++){
            int64_t expected_min = max<int64_t>(1, i); // inclusive
            int64_t expected_max = min<int64_t>(sample_sz, j);
//            cout << ">>>> {min: " << i << ", max: " << j << ", expected_min: " << expected_min << ", expected_max: " << expected_max << "}\n";

            auto it = pma.find(i, j);
            if(expected_min <= expected_max){
                size_t count = 0;
                int64_t expected_key = expected_min;
                while(it->hasNext()){
                    int64_t expected_value = expected_key * 10;

                    auto e = it->next();
//                    cout << "[" << count << "] got: " << e.first << ", expected: " << expected_key << endl;
                    REQUIRE(e.first == expected_key);
                    REQUIRE(e.second == expected_value);

                    expected_key++;
                    count++;
                }

                // Check we have actually read all the keys
                size_t expected_count = expected_max +1 - expected_min;
//                cout << "count: " << count << ", expected_count: " << expected_count << endl;
                REQUIRE(count == expected_count);
            } else {
                bool has_next = it->hasNext();
//                cout << "{has_next: " << has_next << "}" << endl;
                REQUIRE(has_next == false);
            }
        }
    }
}

TEST_CASE("bulk_loading"){
    PackedMemoryArray pma;

    // random permutation of the values in [1, 32]
    int64_t sample[] = {503, 140, 818, 936, 30, 68, 329, 464, 51, 992, 70, 966, 255, 865,
            550, 450, 851, 246, 1020, 589, 616, 731, 549, 132, 653, 904, 973,
            144, 250, 615, 903, 742, 728, 531, 440, 773, 161, 264, 166, 511, 227,
            793, 402, 586, 995, 194, 688, 960, 1014, 943, 576, 968, 90, 700, 252,
            679, 56, 218, 1013, 103, 433, 880, 210, 86, 242, 3, 1012, 235, 646,
            801, 940, 179, 75, 63, 420, 467, 876, 147, 483, 458, 516, 219, 240,
            954, 821, 99, 974, 655, 444, 765, 321, 517, 396, 429, 352, 47, 515,
            266, 810, 878, 481, 985, 822, 186, 375, 50, 486, 61, 459, 693, 805,
            564, 347, 281, 637, 764, 812, 632, 994, 948, 813, 278, 340, 85, 692,
            778, 100, 487, 723, 963, 685, 345, 236, 946, 713, 74, 387, 88, 736,
            914, 603, 125, 910, 439, 243, 497, 199, 447, 138, 279, 55, 343, 33,
            263, 599, 743, 1003, 676, 540, 322, 6, 567, 577, 401, 896, 664, 324,
            900, 130, 588, 204, 162, 133, 468, 207, 901, 471, 182, 905, 569, 38,
            733, 312, 158, 422, 726, 435, 323, 999, 73, 339, 976, 257, 938, 845,
            993, 660, 12, 867, 681, 374, 212, 950, 129, 72, 942, 137, 274, 965,
            858, 454, 505, 656, 882, 747, 949, 583, 617, 841, 213, 37, 513, 873,
            630, 814, 305, 276, 687, 913, 831, 591, 803, 899, 154, 665, 708, 241,
            421, 34, 338, 1001, 492, 258, 41, 496, 370, 81, 299, 197, 790, 473,
            449, 111, 499, 60, 365, 18, 738, 320, 544, 889, 662, 786, 426, 689,
            307, 408, 737, 24, 807, 23, 919, 678, 683, 547, 491, 108, 170, 788,
            233, 1019, 261, 915, 633, 758, 772, 877, 122, 581, 631, 766, 203,
            898, 884, 911, 672, 971, 477, 972, 25, 446, 634, 310, 628, 654, 643,
            590, 143, 933, 727, 395, 970, 127, 150, 855, 355, 341, 1018, 977,
            575, 165, 164, 670, 837, 937, 348, 587, 512, 351, 641, 247, 572, 11,
            755, 605, 479, 44, 975, 1011, 939, 40, 595, 76, 804, 917, 301, 885,
            827, 791, 997, 514, 293, 894, 519, 610, 406, 272, 177, 357, 317, 46,
            128, 106, 874, 951, 991, 785, 337, 437, 17, 673, 160, 852, 771, 328,
            762, 1000, 696, 890, 570, 175, 953, 493, 824, 721, 472, 546, 238,
            330, 908, 935, 545, 448, 445, 432, 897, 955, 245, 844, 806, 794, 409,
            906, 231, 178, 862, 335, 667, 988, 288, 857, 364, 67, 558, 817, 1016,
            598, 639, 699, 929, 645, 234, 624, 188, 168, 701, 769, 453, 141, 405,
            441, 92, 775, 21, 714, 1022, 123, 623, 860, 358, 119, 825, 702, 172,
            947, 134, 187, 843, 768, 902, 149, 19, 148, 83, 669, 354, 332, 79,
            548, 777, 181, 808, 325, 573, 35, 304, 614, 267, 627, 859, 930, 931,
            116, 412, 711, 697, 732, 752, 956, 413, 529, 286, 980, 206, 300, 360,
            921, 716, 501, 926, 333, 740, 694, 710, 192, 394, 945, 748, 463, 820,
            797, 1024, 883, 625, 366, 282, 155, 1002, 336, 751, 718, 214, 1008,
            411, 152, 767, 368, 39, 783, 907, 91, 102, 508, 707, 315, 10, 269,
            739, 613, 866, 428, 895, 296, 110, 215, 136, 839, 163, 659, 925, 532,
            295, 403, 382, 763, 373, 359, 826, 27, 815, 1023, 523, 848, 262, 920,
            982, 229, 784, 153, 26, 671, 407, 666, 256, 651, 465, 109, 1015, 668,
            753, 342, 105, 112, 798, 69, 378, 4, 924, 418, 833, 983, 836, 174,
            145, 191, 776, 469, 268, 870, 230, 561, 356, 869, 451, 557, 506, 789,
            273, 31, 135, 530, 626, 415, 334, 923, 59, 93, 388, 306, 376, 124,
            52, 556, 13, 704, 680, 674, 193, 29, 494, 854, 484, 1017, 819, 121,
            189, 389, 551, 146, 131, 650, 318, 749, 270, 259, 327, 1, 32, 371,
            619, 521, 535, 745, 49, 1009, 927, 416, 892, 579, 196, 816, 962, 381,
            649, 195, 608, 964, 640, 781, 734, 101, 410, 434, 782, 71, 1004, 95,
            584, 642, 5, 525, 260, 538, 996, 522, 430, 498, 677, 847, 504, 478,
            489, 185, 715, 838, 578, 309, 563, 66, 509, 367, 611, 115, 490, 173,
            460, 225, 239, 800, 780, 248, 565, 292, 404, 380, 607, 363, 759, 835,
            916, 582, 720, 823, 887, 756, 400, 313, 648, 369, 383, 120, 729, 661,
            1007, 54, 283, 909, 888, 87, 485, 566, 104, 222, 802, 392, 171, 82,
            850, 20, 209, 151, 285, 724, 419, 319, 275, 725, 326, 518, 979, 45,
            427, 398, 735, 346, 770, 349, 809, 414, 62, 294, 609, 832, 526, 621,
            190, 536, 796, 853, 417, 534, 604, 652, 682, 644, 36, 568, 730, 114,
            834, 425, 77, 64, 456, 842, 89, 868, 15, 647, 251, 48, 934, 952, 932,
            856, 118, 244, 2, 480, 597, 596, 180, 7, 289, 298, 861, 208, 754,
            316, 741, 201, 533, 875, 97, 879, 237, 684, 98, 216, 618, 107, 156,
            142, 912, 918, 14, 290, 475, 344, 302, 744, 423, 779, 176, 431, 636,
            893, 585, 495, 500, 559, 1021, 220, 443, 846, 502, 580, 384, 712,
            399, 717, 657, 967, 722, 601, 331, 774, 462, 8, 829, 379, 592, 593,
            200, 198, 695, 998, 629, 541, 452, 944, 438, 393, 202, 424, 217, 58,
            350, 606, 553, 891, 620, 987, 638, 957, 461, 224, 555, 872, 881, 594,
            571, 562, 377, 291, 457, 53, 658, 9, 537, 675, 372, 524, 574, 311,
            277, 520, 287, 792, 65, 830, 28, 84, 799, 488, 159, 314, 981, 1005,
            539, 686, 554, 126, 871, 42, 205, 698, 253, 663, 442, 184, 886, 969,
            705, 228, 761, 543, 602, 959, 691, 221, 709, 961, 157, 750, 849, 482,
            362, 280, 989, 386, 94, 1006, 474, 211, 455, 390, 466, 254, 353, 928,
            1010, 139, 169, 167, 528, 436, 622, 552, 760, 183, 22, 922, 57, 864,
            703, 719, 226, 43, 113, 308, 397, 361, 635, 787, 840, 600, 811, 986,
            271, 542, 958, 507, 391, 385, 795, 78, 303, 690, 96, 990, 249, 746,
            612, 941, 80, 828, 232, 560, 265, 510, 297, 706, 757, 984, 284, 470,
            16, 117, 863, 223, 527, 476, 978};
    const size_t sample_sz = sizeof(sample) / sizeof(sample[0]); // 1024
    const size_t batch_sz = 32;
    static_assert(sample_sz % batch_sz == 0, "Batch size should to be a multiple of sample size");
    pair<int64_t, int64_t> batch[batch_sz];

    for(size_t i = 0; i < sample_sz / batch_sz; i++){
        size_t base = i * batch_sz;
        for(size_t j = 0; j < batch_sz; j++){
            batch[j].first = sample[base + j];
            batch[j].second = batch[j].first * 10;
        }

        // load the batch
        pma.load(batch, batch_sz);
//        pma.dump();

        REQUIRE(pma.size() == batch_sz * (i+1));
    }

    // check that all elements are present
    for(int64_t key = 1; key <= sample_sz; key++){
        auto value = pma.find(key);
        REQUIRE(key * 10 == value);
    }
}

TEST_CASE("sum"){
    using Implementation = PackedMemoryArray;
    shared_ptr<Implementation> implementation{ new Implementation{} };

//    size_t sz = 64;
//    for(size_t i = 1; i <= sz; i++){
//        implementation->insert(i, i * 10);
//    }

    // a permutation of the numbers between 1 and 1033
    int64_t sample[] = {543, 805, 74, 79, 250, 685, 580, 447, 86, 116, 299, 122, 1028, 769,
            976, 702, 126, 353, 381, 888, 374, 822, 77, 139, 991, 986, 407, 259,
            905, 183, 98, 286, 15, 360, 242, 924, 331, 919, 175, 33, 3, 435, 506,
            372, 516, 815, 594, 748, 852, 860, 659, 990, 310, 1004, 497, 345,
            614, 303, 526, 632, 394, 401, 972, 964, 671, 49, 933, 9, 679, 903,
            662, 863, 899, 209, 645, 365, 975, 755, 841, 366, 747, 461, 923, 699,
            980, 796, 438, 1019, 636, 112, 697, 655, 240, 158, 935, 878, 994,
            408, 1030, 517, 129, 724, 551, 498, 600, 673, 604, 456, 695, 224,
            376, 17, 648, 323, 823, 713, 117, 450, 589, 23, 694, 913, 134, 267,
            609, 762, 814, 12, 11, 227, 618, 81, 16, 235, 615, 654, 95, 1023,
            579, 606, 334, 807, 458, 828, 352, 206, 371, 111, 775, 464, 746, 165,
            586, 857, 812, 793, 94, 43, 889, 170, 71, 383, 1015, 477, 448, 953,
            308, 395, 593, 318, 432, 29, 239, 205, 123, 521, 522, 55, 154, 361,
            612, 959, 504, 880, 869, 625, 251, 667, 216, 797, 798, 476, 453, 825,
            624, 405, 851, 128, 194, 375, 133, 813, 722, 977, 399, 363, 145, 682,
            119, 473, 930, 562, 764, 967, 234, 678, 338, 605, 215, 868, 367, 786,
            90, 38, 162, 136, 558, 496, 248, 84, 463, 581, 651, 75, 290, 411,
            354, 417, 602, 737, 311, 195, 966, 391, 518, 767, 93, 57, 564, 416,
            356, 350, 220, 811, 948, 4, 916, 835, 849, 243, 177, 288, 474, 954,
            277, 268, 6, 35, 137, 1003, 125, 293, 779, 816, 565, 629, 337, 887,
            494, 182, 124, 788, 283, 621, 834, 444, 479, 539, 54, 931, 818, 327,
            21, 771, 336, 428, 58, 40, 475, 409, 776, 355, 932, 709, 845, 89,
            359, 893, 885, 507, 595, 1020, 120, 820, 657, 821, 870, 388, 683,
            908, 140, 324, 985, 901, 840, 696, 396, 961, 672, 965, 530, 951, 442,
            50, 937, 853, 1, 457, 426, 304, 871, 263, 343, 576, 731, 315, 1021,
            873, 368, 941, 511, 617, 791, 262, 78, 377, 664, 829, 830, 460, 649,
            751, 768, 468, 691, 92, 386, 992, 258, 317, 616, 537, 484, 877, 152,
            45, 270, 236, 275, 431, 47, 499, 859, 803, 726, 445, 525, 218, 725,
            599, 100, 141, 989, 106, 918, 715, 533, 400, 563, 710, 910, 443, 690,
            217, 341, 228, 712, 890, 626, 592, 495, 25, 1001, 446, 906, 166, 393,
            650, 244, 720, 349, 153, 552, 1002, 392, 513, 64, 862, 781, 684, 716,
            284, 281, 601, 385, 173, 635, 997, 900, 210, 634, 200, 437, 429, 570,
            414, 280, 316, 757, 264, 883, 1018, 707, 157, 717, 557, 515, 766,
            742, 603, 692, 1009, 677, 178, 266, 760, 864, 466, 109, 455, 652,
            898, 981, 736, 837, 936, 85, 572, 993, 127, 911, 333, 184, 675, 528,
            674, 307, 510, 362, 826, 824, 150, 151, 488, 598, 465, 289, 608, 643,
            312, 1005, 167, 232, 896, 199, 172, 330, 642, 1031, 514, 665, 87,
            246, 817, 238, 97, 378, 640, 568, 193, 204, 138, 744, 535, 287, 469,
            656, 291, 357, 915, 7, 756, 783, 66, 879, 960, 348, 255, 529, 31,
            221, 547, 189, 44, 384, 571, 962, 810, 459, 963, 83, 110, 14, 329,
            1006, 418, 790, 597, 619, 1007, 279, 800, 186, 104, 256, 5, 53, 269,
            56, 647, 872, 855, 774, 523, 897, 895, 440, 838, 831, 987, 508, 926,
            984, 27, 582, 276, 26, 765, 114, 633, 542, 519, 588, 861, 301, 858,
            390, 761, 847, 943, 978, 403, 2, 76, 135, 1013, 24, 82, 561, 693,
            921, 721, 425, 728, 653, 548, 912, 503, 105, 427, 321, 502, 758, 549,
            666, 196, 88, 52, 819, 41, 143, 292, 983, 934, 836, 480, 688, 223,
            265, 101, 389, 198, 213, 591, 844, 118, 947, 300, 611, 806, 638, 566,
            550, 708, 839, 380, 260, 909, 369, 146, 569, 532, 644, 161, 925, 340,
            107, 231, 754, 785, 956, 646, 792, 433, 103, 322, 610, 387, 18, 866,
            65, 10, 876, 802, 491, 1032, 296, 854, 434, 735, 843, 833, 531, 113,
            740, 749, 714, 658, 698, 147, 623, 59, 99, 168, 319, 1024, 174, 298,
            160, 573, 902, 988, 917, 554, 534, 320, 778, 946, 422, 130, 730, 48,
            1014, 732, 939, 622, 982, 734, 470, 998, 211, 607, 430, 711, 254,
            784, 449, 185, 285, 28, 505, 574, 197, 297, 567, 342, 22, 544, 187,
            132, 865, 486, 979, 920, 1026, 108, 809, 230, 436, 782, 439, 326,
            344, 192, 536, 1017, 306, 750, 102, 538, 875, 493, 703, 886, 180,
            928, 927, 670, 804, 729, 957, 904, 585, 745, 358, 272, 179, 527, 949,
            524, 273, 481, 958, 639, 164, 867, 881, 313, 181, 364, 63, 462, 1011,
            892, 191, 1012, 471, 950, 91, 328, 441, 67, 739, 247, 973, 596, 669,
            613, 741, 641, 73, 482, 995, 19, 970, 590, 555, 808, 346, 660, 148,
            294, 397, 155, 706, 668, 794, 752, 188, 974, 131, 1033, 229, 556,
            339, 631, 249, 62, 546, 219, 309, 34, 1025, 509, 208, 176, 743, 545,
            225, 676, 424, 121, 489, 347, 413, 332, 237, 302, 780, 795, 938, 472,
            850, 575, 553, 305, 214, 421, 907, 1027, 914, 929, 689, 630, 60, 351,
            1008, 945, 370, 222, 500, 955, 540, 20, 763, 190, 520, 212, 8, 1010,
            490, 587, 884, 325, 13, 252, 382, 874, 39, 968, 687, 163, 492, 856,
            373, 202, 637, 80, 952, 415, 680, 801, 169, 1029, 753, 583, 940, 46,
            922, 423, 70, 770, 335, 30, 999, 282, 541, 245, 1016, 142, 487, 257,
            419, 261, 404, 36, 68, 37, 944, 271, 274, 559, 759, 894, 467, 772,
            584, 96, 777, 485, 560, 512, 233, 406, 149, 718, 483, 799, 115, 686,
            705, 451, 842, 882, 156, 1000, 848, 846, 454, 207, 295, 51, 478, 32,
            663, 891, 628, 420, 72, 789, 701, 203, 727, 996, 241, 410, 971, 620,
            69, 452, 501, 661, 226, 827, 719, 201, 773, 159, 704, 942, 171, 738,
            398, 577, 42, 61, 723, 379, 700, 402, 253, 278, 832, 412, 578, 314,
            681, 969, 144, 1022, 787, 627, 733};
    int64_t sz = sizeof(sample) / sizeof(sample[0]); // 1033
    for(size_t i = 0; i < sz; i++){
        implementation->insert(sample[i], sample[i] * 10);
    }

    implementation->build();
    REQUIRE(implementation->size() == sz);

//    implementation->dump();

    for(size_t i = 0; i <= sz + 1; i++){
        for(size_t j = i; j <= sz + 2; j++){
            auto sum = implementation->sum(i, j);
//            cout << "RANGE [" << i << ", " << j << "] result: " << sum << endl;

            if(j <= 0 || i > sz){
                REQUIRE(sum.m_num_elements == 0);
                REQUIRE(sum.m_sum_keys == 0);
                REQUIRE(sum.m_sum_values == 0);
            } else {
                int64_t vmin = std::max<int64_t>(1, i);
                int64_t vmax = std::min<int64_t>(sz, j);

                REQUIRE(sum.m_first_key == vmin);
                REQUIRE(sum.m_last_key == vmax);
                REQUIRE(sum.m_num_elements == (vmax - vmin +1));
                auto expected_sum = /* sum of the first vmax numbers */ (vmax * (vmax +1) /2) - /* sum of the first vmin -1 numbers */ ((vmin -1) * vmin /2);
                REQUIRE(sum.m_sum_keys == expected_sum);
                REQUIRE(sum.m_sum_values == expected_sum * 10);
            }
        }
    }
}