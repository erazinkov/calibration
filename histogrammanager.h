#ifndef HISTOGRAMMANAGER_H
#define HISTOGRAMMANAGER_H

#include <memory>
#include <optional>

#include <TH1.h>
#include <TH2.h>

class HistogramManager
{
public:
    HistogramManager(std::optional<std::string> outputDirectory = std::nullopt);
    std::vector<std::vector<std::shared_ptr<TH1>>> createHistograms(const std::string &histName,
                                                                int nBinsX,
                                                                double xLow,
                                                                double xUp,
                                                                std::vector<int> &idxsGamma,
                                                                std::vector<int> &idxsAlpha) const;
    std::vector<std::shared_ptr<TH1>> createHistograms(const std::string &histName,
                                                                int nBinsX,
                                                                double xLow,
                                                                double xUp,
                                                                std::vector<int> &idxs) const;
    std::vector<std::vector<std::shared_ptr<TH2>>> createHistograms(const std::string &histName,
                                                                int nBinsX,
                                                                double xLow,
                                                                double xUp,
                                                                int nBinsY,
                                                                double yLow,
                                                                double yUp,
                                                                std::vector<int> &idxsGamma,
                                                                std::vector<int> &idxsAlpha) const;
    std::vector<std::shared_ptr<TH2>> createHistograms(const std::string &histName,
                                                                int nBinsX,
                                                                double xLow,
                                                                double xUp,
                                                                int nBinsY,
                                                                double yLow,
                                                                double yUp,
                                                                std::vector<int> &idxs) const;
    void printToPsFile(const std::string &psName,
               std::vector<std::vector<std::shared_ptr<TH1>> > &hists) const;
    void printToPsFile(const std::string &fileName,
                                 std::vector<std::shared_ptr<TH1> > &hists) const;
    void printToPsFile(const std::string &psName,
               std::shared_ptr<TH1> hist) const;
    void saveToRootFile(const std::string &fileName,
               std::shared_ptr<TH1> hist) const;
private:
    std::optional<std::string> outputDirectory_;
};

#endif // HISTOGRAMMANAGER_H
