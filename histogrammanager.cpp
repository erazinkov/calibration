#include "histogrammanager.h"

#include <sstream>

#include <iostream>

#include <TCanvas.h>
#include <TError.h>
#include <TFile.h>

HistogramManager::HistogramManager(std::optional<std::string> outputDirectory) : _outputDirectory{outputDirectory}
{

}

std::vector<std::vector<std::shared_ptr<TH1> > > HistogramManager::createHistograms(const std::string &histName,
                                                                                    int nBinsX,
                                                                                    double xLow,
                                                                                    double xUp,
                                                                                    std::vector<int> &idxsGamma,
                                                                                    std::vector<int> &idxsAlpha) const
{
    std::vector<std::vector<std::shared_ptr<TH1>>> hists;
    hists.resize(idxsGamma.size());
    std::stringstream ss;
    for (size_t i{0}; i < idxsGamma.size(); ++i)
    {
        for (size_t j{0}; j <  idxsAlpha.size(); ++j)
        {
            ss.clear();ss.str("");
            ss << histName << "_" << idxsGamma.at(i) << "_" << idxsAlpha.at(j);
            auto h{std::make_shared<TH1D>(ss.str().c_str(), ss.str().c_str(), nBinsX, xLow, xUp)};
            h->Sumw2();
            hists.at(i).push_back(h);
        }
    }
    return hists;
}

std::vector<std::shared_ptr<TH1> > HistogramManager::createHistograms(const std::string &histName,
                                                                      int nBinsX,
                                                                      double xLow,
                                                                      double xUp,
                                                                      std::vector<int> &idxs) const
{
    std::vector<std::shared_ptr<TH1>> hists;
    std::stringstream ss;
    for (size_t i{0}; i < idxs.size(); ++i)
    {
        ss.clear();ss.str("");
        ss << histName << "_" << idxs.at(i);
        auto h{std::make_shared<TH1D>(ss.str().c_str(), ss.str().c_str(), nBinsX, xLow, xUp)};
        h->Sumw2();
        hists.push_back(h);
    }
    return hists;
}

std::vector<std::vector<std::shared_ptr<TH2> > > HistogramManager::createHistograms(const std::string &histName,
                                                                                    int nBinsX,
                                                                                    double xLow,
                                                                                    double xUp,
                                                                                    int nBinsY,
                                                                                    double yLow,
                                                                                    double yUp,
                                                                                    std::vector<int> &idxsGamma,
                                                                                    std::vector<int> &idxsAlpha) const
{
    std::vector<std::vector<std::shared_ptr<TH2>>> hists;
    hists.resize(idxsGamma.size());
    std::stringstream ss;
    for (size_t i{0}; i < idxsGamma.size(); ++i)
    {
        for (size_t j{0}; j <  idxsAlpha.size(); ++j)
        {
            ss.clear();ss.str("");
            ss << histName << "_" << idxsGamma.at(i) << "_" << idxsAlpha.at(j);
            auto h{std::make_shared<TH2D>(ss.str().c_str(), ss.str().c_str(), nBinsX, xLow, xUp, nBinsY, yLow, yUp)};
            h->Sumw2();
            hists.at(i).push_back(h);
        }
    }
    return hists;
}

std::vector<std::shared_ptr<TH2> > HistogramManager::createHistograms(const std::string &histName,
                                                                      int nBinsX,
                                                                      double xLow,
                                                                      double xUp,
                                                                      int nBinsY,
                                                                      double yLow,
                                                                      double yUp,
                                                                      std::vector<int> &idxs) const
{
    std::vector<std::shared_ptr<TH2>> hists;
    std::stringstream ss;
    for (size_t i{0}; i < idxs.size(); ++i)
    {
        ss.clear();ss.str("");
        ss << histName << "_" << idxs.at(i);
        auto h{std::make_shared<TH2D>(ss.str().c_str(), ss.str().c_str(), nBinsX, xLow, xUp, nBinsY, yLow, yUp)};
        h->Sumw2();
        hists.push_back(h);
    }
    return hists;
}

void HistogramManager::printToPsFile(const std::string &fileName,
                             std::vector<std::shared_ptr<TH1> > &hists) const
{
    const std::string psName{(_outputDirectory.has_value() ? (_outputDirectory.value() + "/") : " ") + fileName + ".ps"};
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Print((psName + '[').c_str());
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        hists.at(ig).get()->Draw();
        auto listOfFunctions{hists.at(ig).get()->GetListOfFunctions()};
        for (auto *item : *listOfFunctions)
        {
            item->Draw("SAME");
        }
        c.get()->Print(psName.c_str());
    }
    c.get()->Print((psName + ']').c_str());
    gErrorIgnoreLevel = 0;
}

void HistogramManager::printToPsFile(const std::string &fileName,
                             std::vector<std::vector<std::shared_ptr<TH1> > > &hists) const
{
    const std::string psName{(_outputDirectory.has_value() ? (_outputDirectory.value() + "/") : " ") + fileName + ".ps"};
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Print((psName + '[').c_str());
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        auto cd{static_cast<int>(std::ceil(std::sqrt(hists.at(ig).size())))};
        c.get()->Divide(cd, cd);
        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
        {
            c.get()->cd(static_cast<int>(ia) + 1);
            hists.at(ig).at(ia).get()->Draw();
            auto listOfFunctions{hists.at(ig).at(ia).get()->GetListOfFunctions()};
            for (auto *item : *listOfFunctions)
            {
                item->Draw("SAME");
            }
        }
        c.get()->Print(psName.c_str());
        c.get()->Clear();
    }
    c.get()->Print((psName + ']').c_str());
    c.get()->Delete();
    gErrorIgnoreLevel = 0;
}

//void HistogramManager::printToPsFile(const std::string &fileName,
//                             std::vector<std::vector<std::shared_ptr<TH1> > > &hists) const
//{
//    const std::string psName{(_outputDirectory.has_value() ? (_outputDirectory.value() + "/") : " ") + fileName + ".ps"};
//    gErrorIgnoreLevel = 3'000;
//    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
//    c.get()->Print((psName + '[').c_str());
//    for (size_t ig{0}; ig < hists.size(); ++ig)
//    {
//        auto cd{static_cast<int>(std::ceil(std::sqrt(hists.at(ig).size())))};
//        c.get()->Divide(cd, cd);
//        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
//        {
//            c.get()->cd(static_cast<int>(ia) + 1);
//            hists.at(ig).at(ia).get()->Draw();
//            auto listOfFunctions{hists.at(ig).at(ia).get()->GetListOfFunctions()};
//            for (auto *item : *listOfFunctions)
//            {
//                item->Draw("SAME");
//            }
//        }
//        c.get()->Print(psName.c_str());
//        c.get()->Clear();
//    }
//    c.get()->Print((psName + ']').c_str());
//    gErrorIgnoreLevel = 0;
//}

void HistogramManager::printToPsFile(const std::string &fileName, std::shared_ptr<TH1> hist) const
{
    const std::string psName{(_outputDirectory.has_value() ? (_outputDirectory.value() + "/") : " ") + fileName + ".ps"};
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Print((psName + '[').c_str());
    hist.get()->Draw();
    auto listOfFunctions{hist->GetListOfFunctions()};
    for (auto *item : *listOfFunctions)
    {
        item->Draw("SAME");
    }
    c.get()->Print(psName.c_str());
    c.get()->Print((psName + ']').c_str());
    gErrorIgnoreLevel = 0;
}

void HistogramManager::saveToRootFile(const std::string &fileName, std::shared_ptr<TH1> hist) const
{
    const std::string rootName{(_outputDirectory.has_value() ? (_outputDirectory.value() + "/") : " ") + fileName + ".root"};
    std::unique_ptr<TFile> file{TFile::Open(rootName.c_str(), "RECREATE")};
    if (file.get()->IsOpen())
    {
        hist.get()->Write(hist->GetName(), TObject::kOverwrite);
    }
}
