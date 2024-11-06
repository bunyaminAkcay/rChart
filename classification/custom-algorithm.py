import pandas as pd
import numpy as np
import glob
from scipy.ndimage import gaussian_filter
from PIL import Image
import matplotlib.pyplot as plt

np.set_printoptions(edgeitems=8)

def getData(folderName):
    csv_files = glob.glob( folderName+'/*.csv')
    dataframes = [pd.read_csv(file, header=None) for file in csv_files]
    data = pd.concat(dataframes, ignore_index=True)
    data = data.drop(data.columns[-1], axis=1).dropna()
    return data

def getLabelsAndFeatures(data, maxDensity):
    data = data.iloc[:,:].values
    rawDataLen = len(data)
    rowsToKeep = np.all(data <= maxDensity, axis=1)
    data = data[rowsToKeep]
    filteredDataLen = len(data)
    print("info: " + str(rawDataLen - filteredDataLen) + " data is removed due to have values more than maxDensity.")
    data = data[data[:, 0] != 0]
    print("info: " + str(filteredDataLen - len(data)) + " data that is zero labeled is removed.")
    labels = data[:,0]
    features = data[:, 1:-1].astype(int)
    return labels, features

def seperateUpperLowerPart(features, inputSize):
    halfSize = inputSize//2
    lowerPart = features[:, :halfSize]
    lowerPart = lowerPart[:, ::-1]
    upperPart = features[:, halfSize:]

    return lowerPart, upperPart

maxDensity = 512
inputSize = 1024
assert inputSize%2 == 0

f = np.full((inputSize//2, maxDensity), 0.5)

trainData = getData("data")
testData = getData("test-data")
labels, features = getLabelsAndFeatures(trainData, maxDensity)
testLabels, testFeatures = getLabelsAndFeatures(testData, maxDensity)

lowerFeatures, upperFeatures = seperateUpperLowerPart(features, inputSize)
testLowerFeatures, testUpperFeatures = seperateUpperLowerPart(testFeatures, inputSize)

epochSize = len(features)
epochCount = 100
learningRateMax = 0.05
learningRateMin = 0.01
oneOverEpochSize = 1/epochSize
rolling = True
maxEpoch = len(lowerFeatures)//epochSize
testEpochRatio = 1#50
blurSigma = 1

trainAccuracyResults = []
testAccuracyResults = []
accuracyEpochs = []

for i in range(epochCount):
    print("Epoch " + str(i+1))
    index = i
    testing = False
    if(i%testEpochRatio == testEpochRatio -1):
        testing = True
    
    if (i >= maxEpoch):
        if(rolling):
            i = i % maxEpoch
        else:
            print("info: Reached the max epoch")
            break
    
    startIndex = epochSize * i

    iLowerFeatures = lowerFeatures[startIndex:startIndex+epochSize, :]
    iUpperFeatures = upperFeatures[startIndex:startIndex+epochSize, :]
    iLabels = labels[startIndex:startIndex+epochSize]

    delta = np.full((inputSize//2, maxDensity), 0.0)

    for j in range(epochSize):
        r = 0
        for d in range(inputSize//2):
            x1 = iUpperFeatures[j, d]
            x2 = iLowerFeatures[j, d]
            r += f[d, x1] - f[d, x2]

        rd = iLabels[j]
        p = 0

        if (rd > 0 and r <= 0): 
            p = oneOverEpochSize
        elif (rd < 0 and r >= 0):
            p = -oneOverEpochSize
        
        for d in range(inputSize//2):
            x1 = iUpperFeatures[j, d]
            x2 = iLowerFeatures[j, d]
            delta[d, x1] += p
            delta[d, x2] -= p

    learningRate = (learningRateMax-learningRateMin)*((epochCount - index - 1)/epochCount) + learningRateMin
    
    f = f + learningRate * delta
    f = f / np.max(f)
    #blur
    f = gaussian_filter(f, sigma=blurSigma)
    
    #test
    if(testing):
        array = f - np.min(f)
        array = array/np.max(array)
        colormap = plt.get_cmap('viridis')
        colored_array = colormap(array)
        imageArray = (colored_array[:, :, :3] * 255).astype(np.uint8)
        image = Image.fromarray(imageArray)
        image.save('f.png')


    if (testing):
        accuracyEpochs.append(index)
        print("info: testing...")
        truePredictedCount = 0
        totalDataSize = len(lowerFeatures)
        for j in range(totalDataSize):
            r = 0
            rd = labels[j]
            for d in range(inputSize//2):
                x1 = upperFeatures[j, d]
                x2 = lowerFeatures[j, d]
                r += f[d, x1] - f[d, x2]
            if ((r > 0 and rd > 0) or (r < 0 and rd < 0)):
                truePredictedCount += 1
        
        print("accuracy: " + str(truePredictedCount/totalDataSize))
        trainAccuracyResults.append(truePredictedCount/totalDataSize)
    
    if (testing):
        truePredictedCount = 0
        totalDataSize = len(testLowerFeatures)
        for j in range(totalDataSize):
            r = 0
            rd = testLabels[j]
            for d in range(inputSize//2):
                x1 = testUpperFeatures[j, d]
                x2 = testLowerFeatures[j, d]
                r += f[d, x1] - f[d, x2]
            if ((r > 0 and rd > 0) or (r < 0 and rd < 0)):
                truePredictedCount += 1
        
        print("test accuracy: " + str(truePredictedCount/totalDataSize))
        testAccuracyResults.append(truePredictedCount/totalDataSize)

plt.plot(accuracyEpochs, trainAccuracyResults, label='train', marker='o')
plt.plot(accuracyEpochs, testAccuracyResults, label='test', marker='s')

plt.xlabel('Epoch count')
plt.ylabel('Accuracy')
plt.title('Train and test accuracy results')

plt.legend()

plt.show()