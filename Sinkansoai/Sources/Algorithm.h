#include <algorithm>



template<class Iter, class PredTpye>
void Sort(Iter&& A, Iter&& B, PredTpye&& Pred)
{
	std::sort(A, B, Pred);
}