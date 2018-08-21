#define FUNCTION_NAME ../SRC/xnornet_bwn
#include <c_ops.h>
struct weights {
w_float(363, 96) layer0;
w_bin_float(2400, 256) layer1;
w_bin_float(2304, 384) layer2;
w_bin_float(3456, 384) layer3;
w_bin_float(3456, 256) layer4;
w_bin_float(9216, 4096) layer5;
w_bin_float(4096, 4096) layer6;
w_float(4096, 1000) layer7;};
_Static_assert(sizeof(struct weights) == 23883168, "");
static void* worker(void *_arg) {
struct thread_arg *arg = _arg;;
struct weights *w = arg->weights;
#ifdef PRINT_TIME
uint64_t t0 = get_time(), t1;
#endif
tensor x = (tensor) {3, {227, 227, 3}, __builtin_assume_aligned(arg->in, 16), {__builtin_assume_aligned(arg->tmp, 16), __builtin_assume_aligned(arg->tmp, 16) + 1204224}, .type=FLOAT};
x = conv2d(x, (tensor) {4, {11, 11, 3, 96}, w->layer0.w, .type=FLOAT}, (tensor) {2, {1, 96}, w->layer0.b}, 4, 4, 2, 2, ACTIVE_RELU, 0, arg->sync); wait(arg, 1); TIME();
x = maxpool(x, 3, 3, 2, 2, 0, arg->sync); wait(arg, 2); TIME();
x = conv2d(x, (tensor) {4, {5, 5, 96, 256}, w->layer1.w, .type=BINARY}, (tensor) {2, {1, 256}, w->layer1.b}, 1, 1, 2, 2, ACTIVE_RELU, 0, arg->sync); wait(arg, 3); TIME();
x = maxpool(x, 3, 3, 2, 2, 0, arg->sync); wait(arg, 4); TIME();
x = conv2d(x, (tensor) {4, {3, 3, 256, 384}, w->layer2.w, .type=BINARY}, (tensor) {2, {1, 384}, w->layer2.b}, 1, 1, 1, 1, ACTIVE_RELU, 0, arg->sync); wait(arg, 5); TIME();
x = conv2d(x, (tensor) {4, {3, 3, 384, 384}, w->layer3.w, .type=BINARY}, (tensor) {2, {1, 384}, w->layer3.b}, 1, 1, 1, 1, ACTIVE_RELU, 0, arg->sync); wait(arg, 6); TIME();
x = conv2d(x, (tensor) {4, {3, 3, 384, 256}, w->layer4.w, .type=BINARY}, (tensor) {2, {1, 256}, w->layer4.b}, 1, 1, 1, 1, ACTIVE_RELU, 0, arg->sync); wait(arg, 7); TIME();
x = maxpool(x, 3, 3, 2, 2, 0, arg->sync); wait(arg, 8); TIME();
x = conv2d(x, (tensor) {4, {6, 6, 256, 4096}, w->layer5.w, .type=BINARY}, (tensor) {2, {1, 4096}, w->layer5.b}, 1, 1, 0, 0, ACTIVE_RELU, 0, arg->sync); wait(arg, 9); TIME();
x = conv2d(x, (tensor) {4, {1, 1, 4096, 4096}, w->layer6.w, .type=BINARY}, (tensor) {2, {1, 4096}, w->layer6.b}, 1, 1, 0, 0, ACTIVE_RELU, 0, arg->sync); wait(arg, 10); TIME();
x = conv2d(x, (tensor) {4, {1, 1, 4096, 1000}, w->layer7.w, .type=FLOAT}, (tensor) {2, {1, 1000}, w->layer7.b}, 1, 1, 0, 0, ACTIVE_NONE, 0, arg->sync); wait(arg, 11); TIME();
return x.data;
}
