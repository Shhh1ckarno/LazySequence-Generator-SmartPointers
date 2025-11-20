import matplotlib.pyplot as plt
import pandas as pd


df = pd.read_csv("results.csv")


plt.figure(figsize=(14, 8))


plt.plot(df["size"], df["raw"], label="Raw pointer", marker='o')
plt.plot(df["size"], df["unique"], label="UniquePtr", marker='o')
plt.plot(df["size"], df["shared"], label="SharedPtr", marker='o')


plt.xlabel("Array size")
plt.ylabel("Time (microseconds)")

plt.ticklabel_format(style='plain', axis='x')
plt.ticklabel_format(style='plain', axis='y')


plt.grid(True, linestyle='--', alpha=0.5)
plt.legend()
plt.title("Performance Comparison: Raw vs UniquePtr vs SharedPtr")

plt.show()
