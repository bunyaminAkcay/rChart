import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
import pandas as pd
import glob
from sklearn.metrics import classification_report, accuracy_score
from torchinfo import summary
import torch.nn.functional as F
import numpy as np

class CompNet(nn.Module):
    def __init__(self):
        super(CompNet, self).__init__()

        self.fc1 = nn.Linear(1025, 12)
        self.fc2 = nn.Linear(12, 3)
        
    def forward(self, x):
        x = x.view(x.size(0), -1)
        x = self.fc1(x)
        x = self.fc2(x)
        return F.softmax(x, dim=1)

class CryptoDataset(Dataset):
    def __init__(self, csv_files):
        dataframes = [pd.read_csv(file, header=None) for file in csv_files]
        self.data = pd.concat(dataframes, ignore_index=True)
        self.data = self.data.drop(self.data.columns[-1], axis=1).dropna()
        print(self.data.shape)
        self.labels, self.features = preprocessFeatures(self.data ,1026, False)
        
        print("features shape: " + str(self.features.shape))
        self.labels = self.labels + 1
    
    def __len__(self):
        # Return the number of samples in the dataset
        return len(self.features)
    
    def __getitem__(self, idx):
        # Return a specific sample (features and corresponding label)
        features = self.features[idx].unsqueeze(0)
        return features, self.labels[idx]


# Training function
def train_model(train_loader, test_loader, model, criterion, optimizer, num_epochs=10):
    model.train()
    for epoch in range(num_epochs):
        epoch_loss = 0.0
        for inputs, labels in train_loader:
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()
            epoch_loss += loss.item()
        print(f'Epoch [{epoch+1}/{num_epochs}], Loss: {epoch_loss/len(train_loader):.4f}')
        test_model2(test_loader, model)

# Testing function
def test_model2(test_loader, model):
    model.eval()
    all_preds = []
    all_labels = []
    totalLoss = 0
    with torch.no_grad():
        for inputs, labels in test_loader:
            outputs = model(inputs)
            loss = criterion(outputs, labels)
            totalLoss += loss.item()
            _, predicted = torch.max(outputs, 1)
            all_preds.extend(predicted.cpu().numpy())
            all_labels.extend(labels.cpu().numpy())
    accuracy = accuracy_score(all_labels, all_preds)

    print(f'Total Accuracy: {accuracy:.4f}')
    print('Total loss:' + str(totalLoss/len(test_loader)))


def test_model(test_loader, model, printReport=True):
    model.eval()
    all_preds = []
    all_labels = []
    with torch.no_grad():
        for inputs, labels in test_loader:
            outputs = model(inputs)
            _, predicted = torch.max(outputs, 1)
            all_preds.extend(predicted.cpu().numpy())
            all_labels.extend(labels.cpu().numpy())
    report = classification_report(all_labels, all_preds, target_names=['Class 0', 'Class 1', 'Class 2'])
    accuracy = accuracy_score(all_labels, all_preds)
    if(printReport):
        print(report)
    print(f'Total Accuracy: {accuracy:.4f}')

# Model saving function
def save_model_as_onnx(model, input_size, onnx_file_path="model.onnx"):
    model.eval()
    dummy_input = torch.randn(1, input_size)
    torch.onnx.export(
        model, dummy_input, onnx_file_path, export_params=True,
        opset_version=11, do_constant_folding=True,
        input_names=['input'], output_names=['output'],
        dynamic_axes={'input': {0: 'batch_size'}, 'output': {0: 'batch_size'}}
    )
    print(f"Model has been saved as {onnx_file_path}")

def preprocessFeatures(data, featureSize, half=False):
    #self.labels = torch.tensor(self.data.iloc[:, 0].values, dtype=torch.long)
    #self.features = torch.tensor(self.data.iloc[:, 1:].values, dtype=torch.float32)
    if(half):
    
        features = data.iloc[:, :].values
        maxValue = 800
        heatmapSize = featureSize -2
        newDataSize = heatmapSize//2 +1 +1
        newFeatures = features[:, :newDataSize]
        
        for i in range(heatmapSize//2):
            newFeatures[:, i+1] += features[:, heatmapSize-i]
        
        newFeatures[:, heatmapSize//2+1] = features[:, -1]

        rows_to_keep = np.all(newFeatures <= maxValue, axis=1)
        
        filteredFeatures = newFeatures[rows_to_keep]
        label = filteredFeatures[:, 0]
        atrFeature = filteredFeatures[:, -1]

        print("max :" + str(np.max(filteredFeatures)) )

        filteredFeatures = filteredFeatures/maxValue
        filteredFeatures[:,-1] = atrFeature
        filteredFeatures[:, 0] = label
        
        labelTensor = torch.tensor(filteredFeatures[:, 0], dtype=torch.long)
        featuresTensor = torch.tensor(filteredFeatures[:, 1:], dtype=torch.float32)
        return labelTensor, featuresTensor
    else:
        features = data.iloc[:, :].values
        maxValue = 800
        heatmapSize = featureSize -2

        rows_to_keep = np.all(features <= maxValue, axis=1)
        
        filteredFeatures = features[rows_to_keep]
        label = filteredFeatures[:, 0]
        atrFeature = filteredFeatures[:, -1]

        print("max :" + str(np.max(filteredFeatures)) )

        filteredFeatures = filteredFeatures/maxValue
        filteredFeatures[:,-1] = atrFeature
        filteredFeatures[:, 0] = label
        
        labelTensor = torch.tensor(filteredFeatures[:, 0], dtype=torch.long)
        featuresTensor = torch.tensor(filteredFeatures[:, 1:], dtype=torch.float32)
        return labelTensor, featuresTensor
        


# Hyperparameters
featureSize = 1025
batch_size = 32
learning_rate = 0.002
num_epochs = 100

# Load data
csv_files = glob.glob('data/*.csv')
dataset = CryptoDataset(csv_files)
train_loader = DataLoader(dataset, batch_size=batch_size, shuffle=True)

test_csv_files = glob.glob('test-data/*.csv')
test_dataset = CryptoDataset(test_csv_files)
test_loader = DataLoader(test_dataset, batch_size=batch_size, shuffle=False)

# Initialize model, criterion, and optimizer
model = CompNet()
class_weights = torch.tensor([1.0, 0.66, 1.0])
criterion = nn.CrossEntropyLoss(weight=class_weights)
optimizer = optim.Adam(model.parameters(), lr=learning_rate)

print("========================")
# Train and test the model
train_model(train_loader, test_loader, model, criterion, optimizer, num_epochs=num_epochs)
test_model(train_loader, model)
test_model(test_loader, model)

# Save model as ONNX
saveAsOnnx = False
if saveAsOnnx:
    save_model_as_onnx(model, input_size=featureSize, onnx_file_path="enhanced_nn_module.onnx")

# Model summary
summary(model, input_size=(batch_size, featureSize))
