void X(destroy_tdc_eigen)(X(tdc_eigen) * F) {
    if (F->n < TDC_EIGEN_BLOCKSIZE) {
        free(F->V);
        free(F->lambda);
    }
    else {
        X(destroy_symmetric_dpr1_eigen)(F->F0);
        X(destroy_tdc_eigen)(F->F1);
        X(destroy_tdc_eigen)(F->F2);
        free(F->z);
    }
    free(F);
}

void X(destroy_tdc_eigen_FMM)(X(tdc_eigen_FMM) * F) {
    if (F->n < TDC_EIGEN_BLOCKSIZE) {
        free(F->V);
        free(F->lambda);
    }
    else {
        X(destroy_symmetric_dpr1_eigen_FMM)(F->F0);
        X(destroy_tdc_eigen_FMM)(F->F1);
        X(destroy_tdc_eigen_FMM)(F->F2);
        free(F->z);
    }
    free(F);
}

size_t X(summary_size_tdc_eigen)(X(tdc_eigen) * F) {
    size_t S = 0;
    if (F->n < TDC_EIGEN_BLOCKSIZE)
        S += sizeof(FLT)*F->n*(F->n+1);
    else {
        S += X(summary_size_symmetric_dpr1_eigen)(F->F0);
        S += X(summary_size_tdc_eigen)(F->F1);
        S += X(summary_size_tdc_eigen)(F->F2);
    }
    return S;
}

size_t X(summary_size_tdc_eigen_FMM)(X(tdc_eigen_FMM) * F) {
    size_t S = 0;
    if (F->n < TDC_EIGEN_BLOCKSIZE)
        S += sizeof(FLT)*F->n*(F->n+1);
    else {
        S += X(summary_size_symmetric_dpr1_eigen_FMM)(F->F0);
        S += X(summary_size_tdc_eigen_FMM)(F->F1);
        S += X(summary_size_tdc_eigen_FMM)(F->F2);
    }
    return S;
}


X(tdc_eigen) * X(sdtdc_eig)(X(symmetric_tridiagonal) * T, X(symmetric_tridiagonal) * S) {
    int n = T->n;
    X(tdc_eigen) * F = malloc(sizeof(X(tdc_eigen)));
    if (n < TDC_EIGEN_BLOCKSIZE) {
        FLT * V = calloc(n*n, sizeof(FLT));
        for (int i = 0; i < n; i++)
            V[i+i*n] = 1;
        FLT * lambda = calloc(n, sizeof(FLT));
        X(symmetric_definite_tridiagonal_eig)(T, S, V, lambda);
        F->V = V;
        F->lambda = lambda;
        F->n = n;
    }
    else {
        int s = n>>1, sign = -1;

        FLT * y = calloc(n, sizeof(FLT));
        y[s-1] = sign;
        y[s] = 1;

        FLT rho = -sign*Y(fabs)(T->b[s-1]);
        FLT sigma = -sign*Y(fabs)(S->b[s-1]);

        X(symmetric_tridiagonal) * T1 = malloc(sizeof(X(symmetric_tridiagonal)));
        FLT * at1 = malloc(s*sizeof(FLT));
        FLT * bt1 = malloc((s-1)*sizeof(FLT));
        for (int i = 0; i < s-1; i++) {
            at1[i] = T->a[i];
            bt1[i] = T->b[i];
        }
        at1[s-1] = T->a[s-1] + sign*Y(fabs)(rho);
        T1->a = at1;
        T1->b = bt1;
        T1->n = s;

        X(symmetric_tridiagonal) * S1 = malloc(sizeof(X(symmetric_tridiagonal)));
        FLT * as1 = malloc(s*sizeof(FLT));
        FLT * bs1 = malloc((s-1)*sizeof(FLT));
        for (int i = 0; i < s-1; i++) {
            as1[i] = S->a[i];
            bs1[i] = S->b[i];
        }
        as1[s-1] = S->a[s-1] + sign*Y(fabs)(sigma);
        S1->a = as1;
        S1->b = bs1;
        S1->n = s;

        X(symmetric_tridiagonal) * T2 = malloc(sizeof(X(symmetric_tridiagonal)));
        FLT * at2 = malloc((n-s)*sizeof(FLT));
        FLT * bt2 = malloc((n-s-1)*sizeof(FLT));
        for (int i = s; i < n-1; i++) {
            at2[i-s+1] = T->a[i+1];
            bt2[i-s] = T->b[i];
        }
        at2[0] = T->a[s] + sign*Y(fabs)(rho);
        T2->a = at2;
        T2->b = bt2;
        T2->n = n-s;

        X(symmetric_tridiagonal) * S2 = malloc(sizeof(X(symmetric_tridiagonal)));
        FLT * as2 = malloc((n-s)*sizeof(FLT));
        FLT * bs2 = malloc((n-s-1)*sizeof(FLT));
        for (int i = s; i < n-1; i++) {
            as2[i-s+1] = S->a[i+1];
            bs2[i-s] = S->b[i];
        }
        as2[0] = S->a[s] + sign*Y(fabs)(sigma);
        S2->a = as2;
        S2->b = bs2;
        S2->n = n-s;

        F->F1 = X(sdtdc_eig)(T1, S1);
        F->F2 = X(sdtdc_eig)(T2, S2);

        FLT * z = calloc(n, sizeof(FLT));
        X(tdmv)('T', 1, F->F1, y, 0, z);
        X(tdmv)('T', 1, F->F2, y+s, 0, z+s);

        FLT * d = malloc(n*sizeof(FLT));
        for (int i = 0; i < s; i++) {
            if (F->F1->n < TDC_EIGEN_BLOCKSIZE)
                d[i] = F->F1->lambda[i];
            else
                d[i] = F->F1->F0->lambda[i];
        }
        for (int i = 0; i < n-s; i++) {
            if (F->F2->n < TDC_EIGEN_BLOCKSIZE)
                d[i+s] = F->F2->lambda[i];
            else
                d[i+s] = F->F2->F0->lambda[i];
        }

        X(symmetric_dpr1) * A = malloc(sizeof(X(symmetric_dpr1)));
        A->d = d;
        A->z = z;
        A->rho = rho;
        A->n = n;
        FLT * zb = malloc(n*sizeof(FLT));
        for (int i = 0; i < n; i++)
            zb[i] = z[i];
        X(symmetric_idpr1) * B = malloc(sizeof(X(symmetric_idpr1)));
        B->z = zb;
        B->sigma = sigma;
        B->n = n;

        F->F0 = X(symmetric_definite_dpr1_eig)(A, B);
        F->n = n;

        X(destroy_symmetric_tridiagonal)(T1);
        X(destroy_symmetric_tridiagonal)(T2);
        X(destroy_symmetric_tridiagonal)(S1);
        X(destroy_symmetric_tridiagonal)(S2);
        X(destroy_symmetric_dpr1)(A);
        X(destroy_symmetric_idpr1)(B);
        F->z = y;
    }
    return F;
}

X(tdc_eigen_FMM) * X(sdtdc_eig_FMM)(X(symmetric_tridiagonal) * T, X(symmetric_tridiagonal) * S) {
    int n = T->n;
    X(tdc_eigen_FMM) * F = malloc(sizeof(X(tdc_eigen_FMM)));
    if (n < TDC_EIGEN_BLOCKSIZE) {
        FLT * V = calloc(n*n, sizeof(FLT));
        for (int i = 0; i < n; i++)
            V[i+i*n] = 1;
        FLT * lambda = calloc(n, sizeof(FLT));
        X(symmetric_definite_tridiagonal_eig)(T, S, V, lambda);
        F->V = V;
        F->lambda = lambda;
        F->n = n;
    }
    else {
        int s = n>>1, sign = -1;

        FLT * y = calloc(n, sizeof(FLT));
        y[s-1] = sign;
        y[s] = 1;

        FLT rho = -sign*Y(fabs)(T->b[s-1]);
        FLT sigma = -sign*Y(fabs)(S->b[s-1]);

        X(symmetric_tridiagonal) * T1 = malloc(sizeof(X(symmetric_tridiagonal)));
        FLT * at1 = malloc(s*sizeof(FLT));
        FLT * bt1 = malloc((s-1)*sizeof(FLT));
        for (int i = 0; i < s-1; i++) {
            at1[i] = T->a[i];
            bt1[i] = T->b[i];
        }
        at1[s-1] = T->a[s-1] + sign*Y(fabs)(rho);
        T1->a = at1;
        T1->b = bt1;
        T1->n = s;

        X(symmetric_tridiagonal) * S1 = malloc(sizeof(X(symmetric_tridiagonal)));
        FLT * as1 = malloc(s*sizeof(FLT));
        FLT * bs1 = malloc((s-1)*sizeof(FLT));
        for (int i = 0; i < s-1; i++) {
            as1[i] = S->a[i];
            bs1[i] = S->b[i];
        }
        as1[s-1] = S->a[s-1] + sign*Y(fabs)(sigma);
        S1->a = as1;
        S1->b = bs1;
        S1->n = s;

        X(symmetric_tridiagonal) * T2 = malloc(sizeof(X(symmetric_tridiagonal)));
        FLT * at2 = malloc((n-s)*sizeof(FLT));
        FLT * bt2 = malloc((n-s-1)*sizeof(FLT));
        for (int i = s; i < n-1; i++) {
            at2[i-s+1] = T->a[i+1];
            bt2[i-s] = T->b[i];
        }
        at2[0] = T->a[s] + sign*Y(fabs)(rho);
        T2->a = at2;
        T2->b = bt2;
        T2->n = n-s;

        X(symmetric_tridiagonal) * S2 = malloc(sizeof(X(symmetric_tridiagonal)));
        FLT * as2 = malloc((n-s)*sizeof(FLT));
        FLT * bs2 = malloc((n-s-1)*sizeof(FLT));
        for (int i = s; i < n-1; i++) {
            as2[i-s+1] = S->a[i+1];
            bs2[i-s] = S->b[i];
        }
        as2[0] = S->a[s] + sign*Y(fabs)(sigma);
        S2->a = as2;
        S2->b = bs2;
        S2->n = n-s;

        F->F1 = X(sdtdc_eig_FMM)(T1, S1);
        F->F2 = X(sdtdc_eig_FMM)(T2, S2);

        FLT * z = calloc(n, sizeof(FLT));
        X(tfmv)('T', 1, F->F1, y, 0, z);
        X(tfmv)('T', 1, F->F2, y+s, 0, z+s);

        FLT * d = malloc(n*sizeof(FLT));
        for (int i = 0; i < s; i++) {
            if (F->F1->n < TDC_EIGEN_BLOCKSIZE)
                d[i] = F->F1->lambda[i];
            else
                d[i] = F->F1->F0->lambda[i];
        }
        for (int i = 0; i < n-s; i++) {
            if (F->F2->n < TDC_EIGEN_BLOCKSIZE)
                d[i+s] = F->F2->lambda[i];
            else
                d[i+s] = F->F2->F0->lambda[i];
        }

        X(symmetric_dpr1) * A = malloc(sizeof(X(symmetric_dpr1)));
        A->d = d;
        A->z = z;
        A->rho = rho;
        A->n = n;
        FLT * zb = malloc(n*sizeof(FLT));
        for (int i = 0; i < n; i++)
            zb[i] = z[i];
        X(symmetric_idpr1) * B = malloc(sizeof(X(symmetric_idpr1)));
        B->z = zb;
        B->sigma = sigma;
        B->n = n;

        F->F0 = X(symmetric_definite_dpr1_eig_FMM)(A, B);
        F->n = n;

        X(destroy_symmetric_tridiagonal)(T1);
        X(destroy_symmetric_tridiagonal)(T2);
        X(destroy_symmetric_tridiagonal)(S1);
        X(destroy_symmetric_tridiagonal)(S2);
        X(destroy_symmetric_dpr1)(A);
        X(destroy_symmetric_idpr1)(B);
        F->z = y;
    }
    return F;
}

void X(tdmv)(char TRANS, FLT alpha, X(tdc_eigen) * F, FLT * x, FLT beta, FLT * y) {
    int n = F->n;
    if (n < TDC_EIGEN_BLOCKSIZE)
        X(gemv)(TRANS, n, n, alpha, F->V, n, x, beta, y);
    else {
        int s = n>>1;
        FLT * z = F->z;
        if (TRANS == 'N') {
            X(dvmv)(TRANS, 1, F->F0, x, 0, z);
            X(tdmv)(TRANS, alpha, F->F1, z, beta, y);
            X(tdmv)(TRANS, alpha, F->F2, z+s, beta, y+s);
        }
        else if (TRANS == 'T') {
            X(tdmv)(TRANS, 1, F->F1, x, 0, z);
            X(tdmv)(TRANS, 1, F->F2, x+s, 0, z+s);
            X(dvmv)(TRANS, alpha, F->F0, z, beta, y);
        }
    }
}

void X(tfmv)(char TRANS, FLT alpha, X(tdc_eigen_FMM) * F, FLT * x, FLT beta, FLT * y) {
    int n = F->n;
    if (n < TDC_EIGEN_BLOCKSIZE)
        X(gemv)(TRANS, n, n, alpha, F->V, n, x, beta, y);
    else {
        int s = n>>1;
        FLT * z = F->z;
        if (TRANS == 'N') {
            X(dfmv)(TRANS, 1, F->F0, x, 0, z);
            X(tfmv)(TRANS, alpha, F->F1, z, beta, y);
            X(tfmv)(TRANS, alpha, F->F2, z+s, beta, y+s);
        }
        else if (TRANS == 'T') {
            X(tfmv)(TRANS, 1, F->F1, x, 0, z);
            X(tfmv)(TRANS, 1, F->F2, x+s, 0, z+s);
            X(dfmv)(TRANS, alpha, F->F0, z, beta, y);
        }
    }
}
