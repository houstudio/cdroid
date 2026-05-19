/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <widget/achart/chart/timechart.h>
namespace cdroid{
/**
 * The time chart rendering class.
 */

TimeChart::TimeChart():LineChart(){
    mStartPoint =0.0/0.0/*NAN*/;
}

TimeChart::TimeChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer)
:LineChart(dataset, renderer){
    mStartPoint =0.0/0.0/*NAN*/;
}

std::string TimeChart::getDateFormat() const{
    return mDateFormat;
}

void TimeChart::setDateFormat(const std::string& format){
    mDateFormat = format;
}

void TimeChart::drawXLabels(const std::vector<double>& xLabels, const std::vector<double>& xTextLabelLocations,
        Canvas& canvas, Paint& paint, int left, int top, int bottom, double xPixelsPerUnit, double minX, double maxX) {
    const int length = xLabels.size();
    if (length > 0) {
        const bool showLabels = mRenderer->isShowLabels();
        const bool showGridY = mRenderer->isShowGridY();
        DateFormat format(getDateFormat(xLabels.at(0), xLabels.at(length - 1)));
        for (int i = 0; i < length; i++) {
            const int64_t label = std::round(xLabels.at(i));
            float xLabel = (float) (left + xPixelsPerUnit * (label - minX));
            if (showLabels) {
                canvas.set_color(mRenderer->getXLabelsColor());
                //canvas.drawLine(xLabel, bottom, xLabel, bottom + mRenderer->getLabelsTextSize() / 3, paint);
                drawLine(canvas, xLabel, bottom, xLabel, bottom + mRenderer->getLabelsTextSize() / 3);
                canvas.stroke();
                drawText(canvas, format.format(label), xLabel,
                         bottom + mRenderer->getLabelsTextSize() * 4 / 3 + mRenderer->getXLabelsPadding(), paint, mRenderer->getXLabelsAngle());
            }
            if (showGridY) {
                canvas.set_color(mRenderer->getGridColor());
                drawLine(canvas,xLabel, bottom,xLabel, top);
                canvas.stroke();
            }
        }
    }
    drawXTextLabels(xTextLabelLocations, canvas, paint, true, left, top, bottom, xPixelsPerUnit, minX, maxX);
}

std::string TimeChart::getDateFormat(double start, double end) const{
    if (!mDateFormat.empty()) {
        return mDateFormat;
    }
    std::string format = "HH:mm:ss";
    const double diff = end - start;
    if (diff > DAY && diff < 5 * DAY) {
        format = "yy/MM/dd hh:mm";
    } else if (diff < DAY) {
        format = "HH:mm:ss";
    }
    return format;
}

std::vector<double> TimeChart::getXLabels(double min, double max, int count) const{
    std::vector<double> result;
    if (!mRenderer->isXRoundedLabels()) {
        if (mDataset->getSeriesCount() > 0) {
            auto series = mDataset->getSeriesAt(0);
            int length = series->getItemCount();
            int intervalLength = 0;
            int startIndex = -1;
            for (int i = 0; i < length; i++) {
                double value = series->getX(i);
                if (min <= value && value <= max) {
                    intervalLength++;
                    if (startIndex < 0) {
                        startIndex = i;
                    }
                }
            }
            if (intervalLength < count) {
                for (int i = startIndex; i < startIndex + intervalLength; i++) {
                    result.push_back(series->getX(i));
                }
            } else {
                float step = (float) intervalLength / count;
                int intervalCount = 0;
                for (int i = 0; i < length && intervalCount < count; i++) {
                    double value = series->getX(std::round(i * step));
                    if (min <= value && value <= max) {
                        result.push_back(value);
                        intervalCount++;
                    }
                }
            }
            return result;
        } else {
            return LineChart::getXLabels(min, max, count);
        }
    }
    if (std::isnan(mStartPoint)) {
        //mStartPoint = min - (min % DAY) + DAY + new Date(std::round(min)).getTimezoneOffset() * 60* 1000;
        mStartPoint = min - std::fmod(min, static_cast<double>(TimeChart::DAY)) + TimeChart::DAY;
    }
    if (count > 25) {
        count = 25;
    }


    const double cycleMath = (max - min) / count;
    if (cycleMath <= 0) {
        return result;
    }
    double cycle = DAY;

    if (cycleMath <= DAY) {
        while (cycleMath < cycle / 2.0) {
            cycle = cycle / 2.0;
        }
    } else {
        while (cycleMath > cycle) {
            cycle = cycle * 2.0;
        }
    }

    double val = mStartPoint - std::floor((mStartPoint - min) / cycle) * cycle;
    int i = 0;
    while (val < max && i++ <= count) {
        result.push_back(val);
        val += cycle;
    }

    return result;
}

std::string TimeChart::getChartType() const{
    return "Time";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
TimeChart::DateFormat::DateFormat(const std::string& fmt_pattern) : mPattern(fmt_pattern) {}

std::string TimeChart::DateFormat::format(std::int64_t timestamp_ms) const {
    std::time_t time_sec = timestamp_ms / 1000;
    std::tm* timeinfo = std::localtime(&time_sec);
    std::ostringstream oss;
    std::string result;
    size_t pos = 0;
    char buf[16];
    
    while (pos < mPattern.length()) {
        if (pos + 1 < mPattern.length() && mPattern[pos] == 'y') {
            int year_digits = 0;
            size_t year_start = pos;
            while (pos < mPattern.length() && mPattern[pos] == 'y') {
                year_digits++;
                pos++;
            }
            if (year_digits == 2) {
                std::strftime(buf, sizeof(buf), "%y", timeinfo);
                result += buf;
            } else if (year_digits >= 4) {
                std::strftime(buf, sizeof(buf), "%Y", timeinfo);
                result += buf;
            }
        } else if (pos + 1 < mPattern.length() && mPattern[pos] == 'M') {
            int month_digits = 0;
            size_t month_start = pos;
            while (pos < mPattern.length() && mPattern[pos] == 'M') {
                month_digits++;
                pos++;
            }
            if (month_digits == 1) {
                std::sprintf(buf, "%d", timeinfo->tm_mon + 1);
                result += buf;
            } else if (month_digits == 2) {
                std::strftime(buf, sizeof(buf), "%m", timeinfo);
                result += buf;
            } else if (month_digits == 3) {
                std::strftime(buf, sizeof(buf), "%b", timeinfo);
                result += buf;
            } else if (month_digits >= 4) {
                std::strftime(buf, sizeof(buf), "%B", timeinfo);
                result += buf;
            }
        }else if (pos + 1 < mPattern.length() && mPattern[pos] == 'd') {
            int day_digits = 0;
            size_t day_start = pos;
            while (pos < mPattern.length() && mPattern[pos] == 'd') {
                day_digits++;
                pos++;
            }
            if (day_digits == 1) {
                std::sprintf(buf, "%d", timeinfo->tm_mday);
                result += buf;
            } else if (day_digits >= 2) {
                std::strftime(buf, sizeof(buf), "%d", timeinfo);
                result += buf;
            }
        } else if (pos + 1 < mPattern.length() && mPattern[pos] == 'H') {
            int hour_digits = 0;
            size_t hour_start = pos;
            while (pos < mPattern.length() && mPattern[pos] == 'H') {
                hour_digits++;
                pos++;
            }
            if (hour_digits == 1) {
                std::sprintf(buf, "%d", timeinfo->tm_hour);
                result += buf;
            } else if (hour_digits >= 2) {
                std::strftime(buf, sizeof(buf), "%H", timeinfo);
                result += buf;
            }
        }  else if (pos + 1 < mPattern.length() && mPattern[pos] == 'h') {
            int hour_digits = 0;
            size_t hour_start = pos;
            while (pos < mPattern.length() && mPattern[pos] == 'h') {
                hour_digits++;
                pos++;
            }
            int hour12 = (timeinfo->tm_hour % 12 == 0) ? 12 : timeinfo->tm_hour % 12;
            if (hour_digits == 1) {
                std::sprintf(buf, "%d", hour12);
                result += buf;
            } else if (hour_digits >= 2) {
                std::sprintf(buf, "%02d", hour12);
                result += buf;
            }
        } else if (pos + 1 < mPattern.length() && mPattern[pos] == 'm') {
            int minute_digits = 0;
            size_t minute_start = pos;
            while (pos < mPattern.length() && mPattern[pos] == 'm') {
                minute_digits++;
                pos++;
            }
            if (minute_digits == 1) {
                std::sprintf(buf, "%d", timeinfo->tm_min);
                result += buf;
            } else if (minute_digits >= 2) {
                char buf[3];
                std::strftime(buf, sizeof(buf), "%M", timeinfo);
                result += buf;
            }
        } else if (pos + 1 < mPattern.length() && mPattern[pos] == 's') {
            int second_digits = 0;
            size_t second_start = pos;
            while (pos < mPattern.length() && mPattern[pos] == 's') {
                second_digits++;
                pos++;
            }
            if (second_digits == 1) {
                std::sprintf(buf, "%d", timeinfo->tm_sec);
                result += buf;
            } else if (second_digits >= 2) {
                char buf[12];
                std::strftime(buf, sizeof(buf), "%S", timeinfo);
                result += buf;
            }
        } else {
            // 非格式字符直接添加
            result += mPattern[pos];
            pos++;
        }
    }    
    return result;
}

}/*endof namespace*/
