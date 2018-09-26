#ifndef INITIALIZATION1D_H
#define INITIALIZATION1D_H


/**
 * \brief 1D CRWCR Algorithm
 */
class Initialization1D
{
public:
	Initialization1D();

	~Initialization1D();

	/**
	 * \brief set image data
	 * \param data 
	 * \param width 
	 * \param height 
	 */
	void setImage(const float* data, int width, int height);

	void setSeedBuffer(unsigned char* buffer);

	/**
	 * \brief run crwcr 1D
	 */
	void process();

	float* getWx();
	float* getWy();

	/**
	 * \brief get initialization result
	 * \return 
	 */
	unsigned char* getInitialization();

private:


	/**
	 * \brief tridiagonal matrix algorithm
	 * \param a :lower
	 * \param b :central
	 * \param c :upper
	 * \param d :right vector
	 * \param x :solution
	 * \param numRow 
	 */
	void TDMA(float* a, float* b, float* c, float* d, float* x, int numRow);

	/**
	 * \brief calculate gaussion weight in row order and column order
	 */
	void calculateWeight();


	/**
	 * \brief calculate gradient for each row and each column
	 */
	void calculateGradient();

	void releaseMemory();

	
	/**
	 * \brief normalize data to [0,1]
	 * \param data 
	 * \param numPixel 
	 */
	void normalize(float* data, size_t numPixel);

	inline bool isSeed(int x, int y) const;
	inline bool isFroegroundSeed(int x, int y);
	inline void setForeground(int x, int y);

	int width_, height_;

	float* image_;

	//0: none label, 1: foreground, 2: background
	unsigned char* seedBuffer_;

	// gaussion weight
	float *rowWeight_, *colWeight_;

	// image gradient
	float *rowGrad_, *colGrad_;
};

#endif // INITIALIZATION1D_H
