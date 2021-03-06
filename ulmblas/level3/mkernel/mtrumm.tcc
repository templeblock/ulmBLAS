#ifndef ULMBLAS_LEVEL3_MKERNEL_MTRUMM_TCC
#define ULMBLAS_LEVEL3_MKERNEL_MTRUMM_TCC 1

#include <ulmblas/level3/ukernel/ugemm.h>
#include <ulmblas/level3/mkernel/mtrumm.h>

namespace ulmBLAS {

template <typename IndexType, typename Alpha, typename T, typename TB>
void
mtrumm(IndexType    mc,
       IndexType    nc,
       const Alpha  &alpha,
       const T      *A_,
       const T      *B_,
       TB           *B,
       IndexType    incRowB,
       IndexType    incColB)
{
    const IndexType MR = BlockSizeUGemm<T>::MR;
    const IndexType NR = BlockSizeUGemm<T>::NR;

    const IndexType mp = (mc+MR-1) / MR;
    const IndexType np = (nc+NR-1) / NR;

    const IndexType mr_ = mc % MR;
    const IndexType nr_ = nc % NR;

    IndexType mr, nr;
    IndexType kc;

    const T Zero(0);

    const T *nextA;
    const T *nextB;

    for (IndexType j=0; j<np; ++j) {
        nr    = (j!=np-1 || nr_==0) ? NR : nr_;
        nextB = &B_[j*mc*NR];


        IndexType ia = 0;
        for (IndexType i=0; i<mp; ++i) {
            mr    = (i!=mp-1 || mr_==0) ? MR : mr_;
            kc    = std::max(mc - i*MR, IndexType(0));
            nextA = &A_[(ia+1)*MR];

            if (i==mp-1) {
                nextA = A_;
                nextB = &B_[(j+1)*mc*NR];
                if (j==np-1) {
                    nextB = B_;
                }
            }

            if (mr==MR && nr==NR) {
                ugemm(kc,
                      alpha, &A_[ia*MR], &B_[j*mc*NR+i*MR*NR],
                      Zero,
                      &B[i*MR*incRowB+j*NR*incColB], incRowB, incColB,
                      nextA, nextB);
            } else {
                // Call the buffered micro kernel
                ugemm(mr, nr, kc,
                      alpha, &A_[ia*MR], &B_[j*mc*NR+i*MR*NR],
                      Zero,
                      &B[i*MR*incRowB+j*NR*incColB], incRowB, incColB,
                      nextA, nextB);
            }
            ia += kc;
        }
    }
}

} // namespace ulmBLAS

#endif // ULMBLAS_LEVEL3_MKERNEL_MTRUMM_TCC
