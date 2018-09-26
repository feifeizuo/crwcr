#ifndef SINGLETON_H
#define SINGLETON_H


/**
 * \brief All parameters for CRWCR algorithm.
 */
struct Parameters
{
	// 1D initialization parameters
	int maxIterations1D = 10;
	float gamma1D = 0.2f;
	float lambda1D = 100.f;
	float foreThreshold = 0.6f;

	// 2D PR parameters
	int maxIterations2D = 10;
	float gamma2D = 0.0006f;
	float lambda2D = 100.f;
	float dt = 0.01f;
};

/**
 * \brief Implementing a Thread-Safe Singleton template with C++11 Using Magic Statics
 * \tparam T 
 */
template <class T>
class Singleton final
{
public:
	static T& GetInstance();

private:
	Singleton() = default;
	~Singleton() = default;

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;
};

template <class T>
T& Singleton<T>::GetInstance()
{
	static T instance;
	return instance;
}
#endif // SINGLETON_H
