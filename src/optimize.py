# =========================
# 1. IMPORT THƯ VIỆN
# =========================
import pandas as pd
import joblib

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler, OneHotEncoder
from sklearn.compose import ColumnTransformer
from sklearn.pipeline import Pipeline
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score

from micromlgen import port


# =========================
# 2. LOAD DATA
# =========================
df = pd.read_csv("../model/data/fire_dataset.csv")


# =========================
# 3. TIỀN XỬ LÝ
# =========================

# Convert timestamp
df['timestamp'] = pd.to_datetime(df['timestamp'], errors='coerce')
df = df.dropna(subset=['timestamp'])

# Lọc dữ liệu hợp lệ
df = df[
    (df['temp'] >= 18) &
    (df['humidity'].between(20, 100)) &
    (df['gas'].between(50, 4095))
].copy()

# Tạo time_period
def categorize_time_period(ts):
    h = ts.hour
    if 5 <= h < 12:
        return 'Morning'
    elif 12 <= h < 17:
        return 'Afternoon'
    else:
        return 'Night'

df['time_period'] = df['timestamp'].apply(categorize_time_period)

# Feature selection
feature_cols = ['temp', 'humidity', 'gas', 'time_period']
X = df[feature_cols]
y = df['label']


# =========================
# 4. TRAIN / TEST SPLIT
# =========================
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42, stratify=y
)


# =========================
# 5. PIPELINE
# =========================
numeric_features = ['temp', 'humidity', 'gas']
categorical_features = ['time_period']

preprocessor = ColumnTransformer([
    ('num', StandardScaler(), numeric_features),
    ('cat', OneHotEncoder(handle_unknown='ignore'), categorical_features)
])

model = RandomForestClassifier(
    n_estimators=300,     
    max_depth=12,          
    random_state=42
)

pipeline = Pipeline([
    ('preprocessor', preprocessor),
    ('model', model)
])

# =========================
# 6. TRAIN
# =========================
pipeline.fit(X_train, y_train)


# =========================
# 7. EVALUATE
# =========================
y_pred = pipeline.predict(X_test)
acc = accuracy_score(y_test, y_pred)
print(f"Accuracy: {acc:.4f}")


# =========================
# 8. SAVE MODEL
# =========================
# Lưu model sklearn
joblib.dump(pipeline, "../model/output/model.pkl")


# =========================
# 9. EXPORT C (.h) CHO ESP32
# =========================
rf_model = pipeline.named_steps["model"]

with open("../model/output/model.h", "w") as f:
    f.write(port(rf_model, class_name="FireDetectionModel"))

print("Đã export model.h cho ESP32")