#include "peakfinder.h"

#include <TF1.h>

PeakFinder::PeakFinder() : _calib{1.0}, _offset{0.0}
{

}

double PeakFinder::getFerrum847PosApprox(TH1 *h, double r)
{
    const double pos{h->GetXaxis()->GetBinCenter(h->GetMaximumBin())};
    const double dL{pos * r / 2.35 * 3.0};
    const double dR{pos * r / 2.35 * 4.0};
    const double p1{h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)) - h->GetBinContent(h->GetXaxis()->FindBin(pos - dL))};
    const double p0{h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)) - p1 * (pos + dR)};

    TF1 f("f","gaus(0) + pol1(3)", pos - dL, pos + dR);
    f.SetParameters(h->GetMaximum() - h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)), pos, r / 2.35 * pos, p0, p1);
    f.SetParLimits(0, 0, h->GetMaximum() - h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)));
    f.SetParLimits(1, pos - dL, pos + dR);
    f.SetParLimits(2, 0.0, h->GetBinCenter(h->GetXaxis()->FindBin(pos + dR)) - h->GetBinCenter(h->GetXaxis()->FindBin(pos - dL)));
    h->Fit("f","RQ");

    return f.GetParameter(1);
}



double PeakFinder::getFerrum847Pos(TH1 *h, double peakPosApp)
{

}

double PeakFinder::getFerrum1238Pos(TH1 *h, double appPos)
{

}

double PeakFinder::getHydrogenPos(TH1 *h, double appPos)
{

    auto ff = [&](double *x, double *par){
        double arg_1{0.0}, arg_2{0.0};
        if (par[2] != 0.0)
        {
            arg_1 = ( x[0] - par[1] ) / par[2];
            arg_2 = ( x[0] - par[1] + getdCh(2223.0 - 2100.0)) / par[2];

        }
        double fitval{
            par[0] * TMath::Exp(-0.5 * arg_1 * arg_1)+
            par[0] * par[3] * TMath::Exp(-0.5 * arg_2 * arg_2)+
            par[4] + par[5] * x[0]
        };

        return fitval;
    };

    double posE{2223}, dLe{220}, dRe{190};

    TF1 f("f", ff, getCh(posE - dLe) - getdCh(dLe), getCh(posE - dLe) + getdCh(dRe), 6);
    f.SetParameter(0, 3000);
    f.SetParameter(1, getCh(posE));
    f.SetParameter(2, getdCh(80));
    f.SetParameter(3, 0.1);
    f.SetParameter(4, 1.0e3);
    f.SetParameter(5, 0.0);
    f.SetParLimits(0, 0, 1.0e5);
    f.SetParLimits(1, getCh(posE) - getdCh(dLe), getCh(posE) + getdCh(dRe));
    f.SetParLimits(2, getdCh(40.0), getdCh(100.0));
    f.SetParLimits(3, 0.01, 0.2);
    f.SetParLimits(4, 0.0, 1.0e5);
    f.SetParLimits(5, -100.0, 0.0);
    h->Fit("f","RQ");
    return f.GetParameter(1);
}

double PeakFinder::getCarbonPos(TH1 *h, double appPos)
{
    double pos{appPos};
    double dL{150};
//    double dR{150};
    TF1 f("f", "gaus(0) + pol2(3)", pos - dL * 0.5, pos + dL);
    double amp{h->GetBinContent(h->GetXaxis()->FindBin(pos)) - h->GetBinContent(h->GetXaxis()->FindBin(pos + dL))};

    f.SetParameters(amp, pos, dL * 0.25, 0.0, 0.0, 0.0);
    f.SetParLimits(0, 0.0, 1.0e5);
    f.SetParLimits(1, pos - dL * 0.5, pos + dL * 0.5);
    f.SetParLimits(2, dL * 0.05, dL);
    f.SetParLimits(3, 0.0, h->GetBinContent(h->GetXaxis()->FindBin(pos)));
    f.SetParLimits(4, 0.0, -1.0 * DBL_MAX);
    h->Fit("f", "RQ");

    return f.GetParameter(1);
}
