#include<cuda_runtime.h>

/**
 * \brief CUDA implementation of highly optimized CRWCR algorithm
 */
namespace CRWCRGPU
{

	int iDivUp(int a, int b)
	{
		return ((a % b) != 0) ? (a / b + 1) : (a / b);
	}


	/**
	 * \brief 
	 * \param wx row weight.
	 * \param grad image gradient.
	 * \param seedBuffer foreground and background label.
	 * \param a 
	 * \param b 
	 * \param c 
	 * \param d 
	 * \param dim 
	 */
	__global__ void rowSweepKernel(float* wx, float* grad, unsigned char* seedBuffer,
		float*a, float* b, float* c, float* d, int2 dim, float gamma, float lambda)
	{
		int x = blockIdx.x * blockDim.x + threadIdx.x;
		int y = blockIdx.y * blockDim.y + threadIdx.y;

		if (x >= dim.x || y >= dim.y) return;

		int rowIdx = x + y * dim.x;
		int colIdx = y + x * dim.y;

		if (x == 0)
		{
			a[colIdx] = -1;
			c[colIdx] = -wx[y * dim.x];
			b[colIdx] = -(c[y] + a[y]);
			d[colIdx] = 0;
		}
		else if (x == dim.x - 1)
		{
			a[colIdx] = -wx[rowIdx];
			c[colIdx] = -1;
			b[colIdx] = -(a[colIdx] + c[colIdx]);
			d[colIdx] = 0;
		}
		else
		{
			a[colIdx] = -wx[rowIdx - 1];
			c[colIdx] = -wx[rowIdx];
			b[colIdx] = -(a[colIdx] + c[colIdx]) + gamma * grad[rowIdx] + (seedBuffer[rowIdx] > 0 ? lambda : 0);
			d[colIdx] = (seedBuffer[rowIdx] == 1 ? lambda : 0);
		}
	}

	/**
	 * \brief 
	 * \param wy 
	 * \param grad 
	 * \param seedBuffer 
	 * \param a 
	 * \param b 
	 * \param c 
	 * \param d 
	 * \param dim 
	 */
	__global__ void columnSweepKernel(float* wy, float* grad, unsigned char* seedBuffer,
		float*a, float* b, float* c, float* d, int2 dim, float gamma, float lambda)
	{
		unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
		unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
		if (x >= dim.x || y >= dim.y) return;

		int rowIdx = x + y * dim.x;

		if (y == 0) 
		{
			a[rowIdx] = -1;
			c[rowIdx] = -wy[x * dim.y];
			b[rowIdx] = -(c[rowIdx] + a[rowIdx]);
			d[rowIdx] = 0;
		}
		else if (y == dim.y - 1) 
		{
			a[rowIdx] = -wy[dim.y - 2 + x * dim.y];
			c[rowIdx] = -1;
			b[rowIdx] = -(a[rowIdx] + c[rowIdx]);
			d[rowIdx] = 0;
		}
		else 
		{
			a[rowIdx] = -wy[y - 1 + x * dim.y];
			c[rowIdx] = -wy[y + x * dim.y];
			b[rowIdx] = -(a[rowIdx] + c[rowIdx]) + gamma * grad[rowIdx] + (seedBuffer[rowIdx] > 0 ? lambda : 0);
			d[rowIdx] = (seedBuffer[rowIdx] == 1 ? lambda : 0);
		}
	}

	/**
	 * \brief use a foreground threshold increase seeds.
	 * \param solution 
	 * \param seedBuffer 
	 * \param dim 
	 */
	__global__ void increaseSeedsKernel(float* solution, unsigned char* seedBuffer, int2 dim, float 
				foreThreshold)
	{
		unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
		unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
		if (x >= dim.x || y >= dim.y) return;

		int idx = x + y * dim.x;

		if (solution[idx] > foreThreshold)
		{
			seedBuffer[idx] = 1;
		}
		 
	}


	/**
	 * \brief 
	 * \param wx 
	 * \param wy 
	 * \param grad 
	 * \param u_n 
	 * \param seedBuffer 
	 * \param a 
	 * \param b 
	 * \param c 
	 * \param d 
	 * \param gamma 
	 * \param dim 
	 */
	__global__ void PRRowKernel(float* wx, float* wy, float* grad,
		float* u_n, unsigned char* seedBuffer, float* a, float* b,
		float* c, float* d,int2 dim,float gamma, float lambda, float dt)
	{
		int x = blockIdx.x * blockDim.x + threadIdx.x;
		int y = blockIdx.y * blockDim.y + threadIdx.y;

		if (x >= dim.x || y >= dim.y) return;

		int rowIdx = x + y * dim.x;
		int colIdx = y + x * dim.y;

		if (x == 0) {
			a[colIdx] = -1;
			c[colIdx] = -wx[y * dim.x];
			b[colIdx] = -(c[y] + a[y]);
		}
		else if (x == dim.x - 1) {
			a[colIdx] = -wx[rowIdx];
			c[colIdx] = -1;
			b[colIdx] = -(a[colIdx] + c[colIdx]);
		}
		else {
			a[colIdx] = -wx[rowIdx - 1];
			c[colIdx] = -wx[rowIdx];
			b[colIdx] = -(a[colIdx] + c[colIdx]) + gamma*grad[rowIdx] + (seedBuffer[rowIdx] > 0 ? lambda : 0) + dt;
		}

		float a_, b_, c_;

		if (y == 0) {
			a_ = -1;
			c_ = -wy[x * dim.y];
			b_ = -(c_ + a_);
			d[colIdx] = (seedBuffer[rowIdx] == 1 ? lambda : 0) -
				(u_n[rowIdx] * b_ + u_n[x + dim.x] * c_) + u_n[rowIdx] * dt;
		}
		else if (y == dim.y - 1) {
			a_ = -wy[dim.y - 2 + x * dim.y];
			c_ = -1;
			b_ = -(a_ + c_);
			d[colIdx] = (seedBuffer[rowIdx] == 1 ? lambda : 0) -
				(u_n[x + (y - 1) * dim.x] * a_ + u_n[rowIdx] * b_) +
				u_n[rowIdx] * dt;
		}
		else {
			a_ = -wy[y + x * dim.y - 1];
			c_ = -wy[y + x * dim.y];
			b_ = -(a_ + c_);
			d[colIdx] = (seedBuffer[rowIdx] == 1 ? lambda : 0) -
				(u_n[x + (y - 1) * dim.x] * a_ + u_n[rowIdx] * b_ +
					u_n[x + (y + 1) * dim.x] * c_) +
				u_n[rowIdx] * dt;
		}
	}

	/**
	 * \brief 
	 * \param wx 
	 * \param wy 
	 * \param grad 
	 * \param u_n 
	 * \param seedBuffer 
	 * \param a 
	 * \param b 
	 * \param c 
	 * \param d 
	 * \param gamma 
	 * \param dim 
	 */
	__global__ void PRColumnKernel(float* wx, float* wy, float* grad, 
		float* u_n, unsigned char* seedBuffer, float* a, float* b, float* c, float* d
		, int2 dim, float gamma, float lambda, float dt)
	{
		unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
		unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
		if (x >= dim.x || y >= dim.y) return;

		int index = x + y * dim.x;

		if (y == 0) {
			a[index] = -1;
			c[index] = -wy[x * dim.y];
			b[index] = -(c[index] + a[index]);
		}
		else if (y == dim.y - 1) {
			a[index] = -wy[dim.y - 2 + x * dim.y];
			c[index] = -1;
			b[index] = -(a[index] + c[index]);
		}
		else {
			a[index] = -wy[y - 1 + x * dim.y];
			c[index] = -wy[y + x * dim.y];
			b[index] = -(a[index] + c[index]) + gamma*grad[index] +	(seedBuffer[index] > 0 ? lambda : 0) + dt;
		}

		float a_, b_, c_;

		if (x == 0) {
			a_ = -1;
			c_ = -wx[index];
			b_ = -(c_ + a_);
			d[index] = (seedBuffer[index] == 1 ? lambda : 0) -
				(u_n[y * dim.x] * b_ + u_n[1 + y * dim.x] * c_) +
				u_n[index] * dt;
		}
		else if (x == dim.x - 1) {
			a_ = -wx[index - 1];
			c_ = -1;
			b_ = -(a_ + c_);
			d[index] = (seedBuffer[index] == 1 ? lambda : 0) -
				(u_n[x - 1 + y * dim.x] * a_ + u_n[x + y * dim.x] * b_) +
				u_n[index] * dt;
		}
		else {
			a_ = -wx[index - 1];
			c_ = -wx[index];
			b_ = -(a_ + c_);
			d[index] = (seedBuffer[index] == 1 ? lambda : 0) -
				(u_n[x - 1 + y * dim.x] * a_ + u_n[x + y * dim.x] * b_ +
					u_n[x + 1 + y * dim.x] * c_) +
				u_n[index] * dt;
		}
	}


	/**
	 * \brief Use TDMA solve tri-dialogal equations. Note that a,b,c,d store in column order.
	 * \param a 
	 * \param b 
	 * \param c 
	 * \param d 
	 * \param solution 
	 * \param dim 
	 */
	__global__ void TDMARow(float* a, float* b, float* c, float* d, float* solution, int2 dim)
	{
		unsigned int y = blockIdx.x * blockDim.x + threadIdx.x;

		if (y >= dim.y) return;

		c[y] = c[y] / b[y];
		d[y] = d[y] / b[y];

		float id;
		int index;
		for (int x = 1; x < dim.x; x++) {
			index = y + x * dim.y;
			id = 1.0 / (b[index] - c[y + (x - 1) * dim.y] * a[index]);
			c[index] = c[index] * id;
			d[index] = (d[index] - a[index] * d[y + (x - 1) * dim.y]) * id;
		}
		index = y + (dim.x - 1) * dim.y;

		solution[dim.x - 1 + y * dim.x] = d[index];

		for (int x = dim.x - 2; x > -1; x--)
		{
			index = y + x * dim.y;
			solution[x + y * dim.x] = d[index] - c[index] * solution[x + 1 + y * dim.x];
		}
	}

	/**
	 * \brief 
	 * \param a 
	 * \param b 
	 * \param c 
	 * \param d 
	 * \param solution 
	 * \param dim 
	 */
	__global__ void TDMAColumn(float* a, float* b, float* c, float* d, float* solution, int2 dim)
	{
		int x = blockIdx.x * blockDim.x + threadIdx.x;
		if (x >= dim.x) return;

		c[x] = c[x] / b[x];
		d[x] = d[x] / b[x];

		float id;
		int index;
		for (int y = 1; y < dim.y; y++) 
		{
			index = x + y * dim.x;
			id = 1.0 / (b[index] - c[x + (y - 1) * dim.x] * a[index]);
			c[index] = c[index] * id;
			d[index] = (d[index] - a[index] * d[x + (y - 1) * dim.x]) * id;
		}

		index = x + (dim.y - 1) * dim.x;
		solution[index] = d[index];

		for (int y = dim.y - 2; y > -1; y--) 
		{
			index = x + y * dim.x;
			solution[index] = d[index] - c[index] * solution[x + (y + 1) * dim.x];
		}
	}
}  // namespace CUDA