float trinetra_compound_hazard_score(float water_score, float trend_score)
{ float x = water_score + trend_score; return x > 100.0f ? 100.0f : x; }
