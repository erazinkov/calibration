#include "peakfinder.h"

#include <TF1.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TVirtualFitter.h>

PeakFinder::PeakFinder() : _calib{1.0}, _offset{0.0}
{

}

void PeakFinder::process(std::vector<TH1 *> &histsSg, std::vector<TH1 *> &histsRc)
{
    TVirtualFitter::SetDefaultFitter("Minuit");

    const std::string psName{"gamma_calib.ps"};
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Divide(2, 2);
    c.get()->Print((psName + '[').c_str());

    auto n{histsSg.size()};

    _f.resize(n, nullptr);

    for (size_t i{0}; i < n; ++i)
    {

        _f.at(i) = new TF1(("fN_" + std::to_string(i)).c_str(), "pol3", 0.0, 8.0e3);
        TGraphErrors graphPolN(5);
        auto fe847PosApprox{getFerrum847PosApprox(histsRc.at(i), 0.12)};

        _offset = 0.0;
        _calib = (847.0 - _offset) / fe847PosApprox;
        auto fe847Pos{getFerrum847Pos(histsRc.at(i))};

        graphPolN.SetPoint(0, fe847Pos, 847.0);
        _calib  = (847.0 - _offset) / fe847Pos;
        auto fe1238Pos{getFerrum1238Pos(histsRc.at(i))};

        graphPolN.SetPoint(1, fe1238Pos, 1238.0);
        _calib  = (1238.0 - _offset) / fe1238Pos;
        auto hydPos{getHydrogenPos(histsRc.at(i))};

        graphPolN.SetPoint(2, hydPos, 2223.0);
        _calib  = (2223.0 - _offset) / hydPos;

        TF1 fApp("fApp", "pol2", 0.0, 8.0e3);
        graphPolN.Fit(&fApp, "RQ0");
        auto carbonPos{getCarbonPos(histsSg.at(i), fApp.GetX(4438.0) - 25)};

        graphPolN.SetPoint(3, carbonPos, 4438.0);
        _calib  = (4438.0 - _offset) / carbonPos;
        graphPolN.Fit(&fApp, "RQ0");
        auto oxygenPos{getOxygenPos(histsSg.at(i), fApp.GetX(6129.0))};

        graphPolN.SetPoint(4, oxygenPos, 6129.0);
        _calib  = (6129.0 - _offset) / oxygenPos;
        auto fe7631Pos{getFerrum7631Pos(histsRc.at(i), 0.0)};
        graphPolN.SetPoint(5, fe7631Pos, 7631.0);
        _f.at(i)->SetParameters(25.0, 2.5, 2.5 * 1e-4, -1.0 * 1e-8);
        graphPolN.Fit(_f.at(i), "RQ");

        c.get()->cd(1);
        histsSg.at(i)->Draw();
//        c.get()->cd(2);
//        histsRc.at(i)->Draw();
    }
    c.get()->Print(psName.c_str());
    c.get()->Print((psName + ']').c_str());
    //    c->Print(psName.c_str());
}

std::vector<std::vector<double> > PeakFinder::getPar()
{
    std::vector<std::vector<double>> par;
    par.resize(_f.size());
    for (size_t i{0}; i < _f.size(); ++i)
    {
        for (auto ip{0}; ip < _f.at(i)->GetNpar(); ++ip)
        {
            par.at(i).push_back(_f.at(i)->GetParameter(ip));
        }
    }
    return par;
}

double PeakFinder::getFerrum847PosApprox(TH1 *h, double r)
{

    double feLow=h->GetXaxis()->GetBinCenter(h->GetMaximumBin()), dfeLow=35, dfeHigh=50, dfeLow2=35, dfeHigh2=50;//rea
    TF1 f("f","gaus(0)+pol1(3)", feLow-dfeLow, feLow+dfeHigh);
    double p0, p1;
    p1 = (h->GetBinContent(h->GetXaxis()->FindBin(feLow+dfeHigh))-h->GetBinContent(h->GetXaxis()->FindBin(feLow-dfeLow)))
            /((feLow+dfeHigh)-(feLow-dfeLow));
    p0 = h->GetBinContent(h->GetXaxis()->FindBin(feLow+dfeHigh))-p1*(feLow+dfeHigh);
        f.SetParameters(h->GetMaximum() - h->GetBinContent(h->GetXaxis()->FindBin(feLow+dfeHigh)),
    feLow,h->GetBinCenter(h->GetXaxis()->FindBin(feLow+dfeHigh)) - h->GetBinCenter(h->GetXaxis()->FindBin(feLow-dfeLow)),p0,p1);

    f.SetParLimits(0, 0, h->GetMaximum() - h->GetBinContent(h->GetXaxis()->FindBin(feLow+dfeHigh)));
    f.SetParLimits(1,feLow-dfeLow,feLow+dfeHigh);
    f.SetParLimits(2,0.,h->GetBinCenter(h->GetXaxis()->FindBin(feLow+dfeHigh)) - h->GetBinCenter(h->GetXaxis()->FindBin(feLow-dfeLow)));
    h->Fit("f","R");
    return f.GetParameter(1);

}

//double PeakFinder::getFerrum847PosApprox(TH1 *h, double r)
//{
//    double pos{h->GetXaxis()->GetBinCenter(h->GetMaximumBin())};
//    double dL{pos * r / 2.35 * 3.0};
//    double dR{pos * r / 2.35 * 4.0};
//    double p1{h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)) - h->GetBinContent(h->GetXaxis()->FindBin(pos - dL))};
//    double p0{h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)) - p1 * (pos + dR)};

//    TF1 f("f","gaus(0) + pol1(3)", pos - dL, pos + dR);
//    f.SetParameters(h->GetMaximum() - h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)), pos, r / 2.35 * pos, p0, p1);
//    f.SetParLimits(0, 0, h->GetMaximum() - h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)));
//    f.SetParLimits(1, pos - dL, pos + dR);
//    f.SetParLimits(2, 0.0, h->GetBinCenter(h->GetXaxis()->FindBin(pos + dR)) - h->GetBinCenter(h->GetXaxis()->FindBin(pos - dL)));
//    h->Fit(&f,"RQ");

//    return f.GetParameter(1);
//}



double PeakFinder::getFerrum847Pos(TH1 *h)
{
    double    feLow=847, dfeLow=100, dfeHigh=175, dfeLow2=95, dfeHigh2=105;//rea
    TF1 f("fFe847","gaus(0)+pol1(3)", getCh(feLow-dfeLow), getCh(feLow+dfeHigh));
    double p0, p1;
    p1 = (h->GetBinContent(h->GetXaxis()->FindBin(getCh(feLow+dfeHigh)))-h->GetBinContent(h->GetXaxis()->FindBin(getCh(feLow-dfeLow))))
            /(getCh(feLow+dfeHigh)-getCh(feLow-dfeLow));
    p0 = h->GetBinContent(h->GetXaxis()->FindBin(getCh(feLow+dfeHigh)))-p1*getCh(feLow+dfeHigh);
    f.SetParameters(1000,getCh(847),getdCh(40),p0,p1); //ryn
    f.SetParLimits(0, 0, 1.0e6);
    f.SetParLimits(1,getCh(800),getCh(900));
    f.SetParLimits(2,getdCh(20.),getdCh(60.));
    f.SetParLimits(4,p1,0);
    h->Fit(&f,"RQ");
    return f.GetParameter(1);
}

double PeakFinder::getFerrum1238Pos(TH1 *h)
{
    double fe=1238, dfe=165, dfe2=145;
    TF1 f("fFe1238","gaus(0)+pol1(3)", getCh(fe-dfe), getCh(fe+dfe));
    f.SetParameters(3000,getCh(fe),getdCh(80),1000,0);
    f.SetParLimits(0,0,1.0e5);
    f.SetParLimits(1,getCh(fe-100),getCh(fe+100));
    f.SetParLimits(2,getdCh(50.),getdCh(120.));
    h->Fit(&f,"RQ");

    return f.GetParameter(1);
}

double PeakFinder::getHydrogenPos(TH1 *h)
{

    double posE{2223}, dLe{220}, dRe{190};

    TF1 f("f","gaus(0)+pol1(3)", getCh(posE-dLe), getCh(posE+dRe));
    f.SetParameters(3000,getCh(posE),getdCh(80),1000,0);
    f.SetParLimits(0,0,1.0e5);
    f.SetParLimits(1,getCh(posE-100),getCh(posE+100));
    f.SetParLimits(2,getdCh(50.),getdCh(120.));
    h->Fit(&f,"RN0");


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


    TF1 f1("f1", ff, f.GetParameter(1) - getdCh(dLe), f.GetParameter(1) + getdCh(dRe), 6);


    f1.SetParameter(0,f.GetParameter(0));
    f1.SetParameter(1,f.GetParameter(1));
    f1.SetParameter(2,f.GetParameter(2));
    f1.SetParameter(3,0.1);
    f1.SetParameter(4,  f.GetParameter(3));
    f1.SetParameter(5,f.GetParameter(4));
    f1.SetParLimits(0,0,1.0e5);
    f1.SetParLimits(1,f.GetParameter(1)-getdCh(dLe),f.GetParameter(1)+getdCh(dRe));
    f1.SetParLimits(2,getdCh(40.),getdCh(100.));
    f1.SetParLimits(3,0.01,0.2);
    f1.SetParLimits(4,0.,1.0e5);
    f1.SetParLimits(5,-100.,0.);
    h->Fit(&f1,"RQ");

    return f1.GetParameter(1);
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

double PeakFinder::getOxygenPos(TH1 *h, double appPos)
{
    double pos{appPos};
    double dL{350.0};
    double dR{200.0};
    double p1 = (h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)) - h->GetBinContent(h->GetXaxis()->FindBin(pos - dL)))
            / (dR + dL);
    double p0{h->GetBinContent(h->GetXaxis()->FindBin(pos + dR)) - p1 * pos};

    TF1 f("f","gaus(0) + gaus(3) + pol1(6)", pos - dL, pos + dR);
    f.SetParameters(1000, pos - getdCh(500), getdCh(80), 3000, pos, getdCh(80), p0, p1);
    f.SetParameter(0, h->GetBinContent(h->GetXaxis()->FindBin(f.GetParameter(1))));
    f.SetParameter(3, h->GetBinContent(h->GetXaxis()->FindBin(f.GetParameter(3))));
    f.SetParLimits(0, 0.0, 1.0e5);
    f.SetParLimits(1, getCh(5350.0), getCh(5800.0));
    f.SetParLimits(2, getdCh(50.0), getdCh(120.0));
    f.SetParLimits(3, 0, 1.0e5);
    f.SetParLimits(4, getCh(5900.0), getCh(6450.0));
    f.SetParLimits(5, getdCh(50.0), getdCh(120.0));
    h->Fit("f","RQ0");
    f.SetRange(f.GetParameter(1) - 3.0 * f.GetParameter(2), f.GetParameter(4) + 3.0 * f.GetParameter(5));
    h->Fit("f","RQ");
    return f.GetParameter(4);
}

double PeakFinder::getFerrum7631Pos(TH1 *h, double appPos)
{
    double fc=7631, dfc1=800, dfc2=370;
    double corr=-350;
    TF1 f("f","gaus(0)+gaus(3)+pol1(6)", getCh(fc-dfc1+corr), getCh(fc+dfc2+corr)); //ryn
    double p0;
    double p1{(h->GetBinContent(h->GetXaxis()->FindBin(getCh(fc + dfc2)))-h->GetBinContent(h->GetXaxis()->FindBin(getCh(fc-dfc1))))
                /(getCh(fc + dfc2) - getCh(fc - dfc1))};
    p0 = h->GetBinContent(h->GetXaxis()->FindBin(getCh(fc+dfc2)))-p1*getCh(fc+dfc2);
    f.SetParameters(1000,getCh(7100+corr),getdCh(80),500,getCh(7631+corr),getdCh(80),p0,p1);
    f.SetParLimits(0,0,1.0e5);
    f.SetParLimits(1,getCh(6900+corr),getCh(7300+corr));
    f.SetParLimits(2,getdCh(50.),getdCh(120.));
    f.SetParLimits(3,0,1.0e5);
    f.SetParLimits(4,getCh(7400+corr),getCh(7850+corr));
    f.SetParLimits(5,getdCh(50.),getdCh(120.));
    h->Fit("f","RQN0");
    f.SetRange(f.GetParameter(4)-getdCh(dfc1),f.GetParameter(4)+getdCh(dfc2));
    h->Fit("f","RQ");
    return f.GetParameter(4);
}
