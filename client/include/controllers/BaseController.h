/**
 * @file BaseController.h
 * @brief 控制器基类 - 所有控制器的公共抽象类
 * @author [pengchengkang]
 * date 2025.06.19
 */
#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

#include <string>

class BaseController {
public:
    BaseController();
    virtual ~BaseController() = default;

    virtual void Init() = 0;
    virtual void Update(float dt) = 0;
    virtual void HandleInput() = 0;

    const std::string& GetName() const;

protected:
    std::string name_;
};

#endif // BASE_CONTROLLER_H